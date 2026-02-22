#include "Map/Rendering/VisualizationManager.hpp"

#include <QDebug>

using namespace ROBOGait::map::rendering;

VisualizationManager::VisualizationManager(QObject* parent) :
    QObject(parent), parent_node_(nullptr), update_timer_(nullptr), scene_(nullptr), is_initialized_(false), render_requested_(false)
{
  qInfo() << "[VisualizationManager] Visualization Manager created";

  // Create graphics scene
  scene_ = std::make_unique<QGraphicsScene>();

  // Create update timer
  update_timer_ = new QTimer(this);

  // clang-format off
  connect(update_timer_,
          &QTimer::timeout,
          this,
          &VisualizationManager::onUpdateTimer);
  // clang-format on
}

VisualizationManager::~VisualizationManager()
{
  stopUpdate();

  // Shutdown all displays
  for (auto& pair : display_map_)
  {
    if (pair.second)
    {
      pair.second->shutdown();
    }
  }
  display_map_.clear();

  qInfo() << "[VisualizationManager]  Visualization Manager destroyed";
}

void VisualizationManager::initialize(rclcpp::Node* parent_node)
{
  if (is_initialized_)
  {
    qWarning() << "[VisualizationManager::initialize] Already initialized";
    return;
  }

  if (!parent_node)
  {
    qCritical() << "[VisualizationManager::initialize] Null parent_node pointer";
    return;
  }

  parent_node_ = parent_node;
  is_initialized_ = true;

  qInfo() << "[VisualizationManager::initialize] Initialized with node:" << QString::fromStdString(parent_node_->get_name());
}

void VisualizationManager::startUpdate()
{
  if (!is_initialized_)
  {
    qWarning() << "[VisualizationManager::startUpdate] Not initialized, call initialize() first";
    return;
  }

  if (update_timer_->isActive())
  {
    qWarning() << "[VisualizationManager::startUpdate] Update loop already running";
    return;
  }

  // Initialize timestamps
  last_update_ = std::chrono::steady_clock::now();
  last_render_ = last_update_;

  // Start timer at 30 Hz
  update_timer_->start(UPDATE_RATE_MS);

  qInfo() << "[VisualizationManager::startUpdate] Update loop started at" << (1000.0 / UPDATE_RATE_MS) << "Hz";
  emit updateStarted();
}

void VisualizationManager::stopUpdate()
{
  if (update_timer_ && update_timer_->isActive())
  {
    update_timer_->stop();
    qInfo() << "[VisualizationManager::stopUpdate] Update loop stopped";
    emit updateStopped();
  }
}

void VisualizationManager::addDisplay(const std::string& name, std::shared_ptr<ROBOGait::map::display::BaseDisplay> display)
{
  if (!display)
  {
    qWarning() << "[VisualizationManager::addDisplay] Null display pointer";
    return;
  }

  // Check if display with same name already exists
  if (display_map_.find(name) != display_map_.end())
  {
    qWarning() << "[VisualizationManager::addDisplay] Display with name" << QString::fromStdString(name) << "already exists";
    return;
  }

  display_map_[name] = display;

  // Add graphics item to scene if display has one
  QGraphicsItem* graphics_item = display->getGraphicsItem();
  if (graphics_item && scene_)
  {
    scene_->addItem(graphics_item);
    qInfo() << "[VisualizationManager::addDisplay] Graphics item added to scene for display:" << QString::fromStdString(name);
  }

  if (is_initialized_)
  {
    display->initialize(parent_node_);
  }

  // Connect render request signal
  // clang-format off
  connect(display.get(),
          &ROBOGait::map::display::BaseDisplay::renderRequested,
          this,
          &VisualizationManager::onRenderRequested);
  // clang-format on

  qInfo() << "[VisualizationManager::addDisplay] Added display:" << QString::fromStdString(name);
}

void VisualizationManager::removeDisplay(const std::string& name)
{
  auto it = display_map_.find(name);

  if (it != display_map_.end())
  {
    // Remove graphics item from scene
    QGraphicsItem* graphics_item = it->second->getGraphicsItem();
    if (graphics_item && scene_)
    {
      scene_->removeItem(graphics_item);
    }

    it->second->shutdown();
    display_map_.erase(it);
    qInfo() << "[VisualizationManager::removeDisplay] Removed display:" << QString::fromStdString(name);
  }
  else
  {
    qWarning() << "[VisualizationManager::removeDisplay] Display not found:" << QString::fromStdString(name);
  }
}

void VisualizationManager::removeAllDisplays()
{
  qInfo() << "[VisualizationManager::removeAllDisplays] Removing all displays (" << display_map_.size() << "total)";

  for (auto& pair : display_map_)
  {
    // Remove graphics item from scene
    QGraphicsItem* graphics_item = pair.second->getGraphicsItem();
    if (graphics_item && scene_)
    {
      scene_->removeItem(graphics_item);
    }

    pair.second->shutdown();
  }

  display_map_.clear();

  qInfo() << "[VisualizationManager::removeAllDisplays] All displays removed";
}

std::shared_ptr<ROBOGait::map::display::BaseDisplay> VisualizationManager::getDisplay(const std::string& name)
{
  auto it = display_map_.find(name);

  if (it != display_map_.end())
  {
    return it->second;
  }

  return nullptr;
}

QGraphicsScene* VisualizationManager::getScene() const { return scene_.get(); }

void VisualizationManager::queueRender()
{
  QMutexLocker lock(&render_mutex_);
  render_requested_ = true;
}

void VisualizationManager::onUpdateTimer()
{
  // Calculate delta time
  auto now = std::chrono::steady_clock::now();
  auto wall_dt = std::chrono::duration<double>(now - last_update_).count();
  last_update_ = now;

  // TODO: for now, same as wall time - can be improved later
  double ros_dt = wall_dt;

  // Update all displays
  updateDisplays(wall_dt, ros_dt);

  // Check if we should render
  bool should_render = false;
  {
    QMutexLocker lock(&render_mutex_);
    should_render = render_requested_;
    render_requested_ = false;
  }

  // Force render if too much time has passed
  auto time_since_render = std::chrono::duration<double>(now - last_render_).count();
  if (time_since_render > FORCE_RENDER_INTERVAL_SEC)
  {
    should_render = true;
  }

  if (should_render)
  {
    renderFrame();
    last_render_ = now;
  }
}

void VisualizationManager::onRenderRequested() { queueRender(); }

void VisualizationManager::updateDisplays(double wall_dt, double ros_dt)
{
  for (auto& pair : display_map_)
  {
    if (pair.second && pair.second->isEnabled())
    {
      pair.second->update(wall_dt, ros_dt);
    }
  }
}

void VisualizationManager::renderFrame()
{
  QMutexLocker lock(&render_mutex_);

  // Calculate FPS
  static auto last_fps_calc = std::chrono::steady_clock::now();
  static int frame_count = 0;
  frame_count++;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration<double>(now - last_fps_calc).count();

  double fps = 0.0;
  if (elapsed >= 1.0)
  {
    fps = frame_count / elapsed;
    frame_count = 0;
    last_fps_calc = now;
  }

  // Emit signal (will trigger widget repaint)
  emit frameRendered(fps);
}
