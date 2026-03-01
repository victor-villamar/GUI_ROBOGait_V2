#include <QDebug>

#include "Map/Rendering/RenderPipeline.hpp"

using namespace ROBOGait::map::rendering;

RenderPipeline::RenderPipeline(QObject* parent) : QObject(parent), update_timer_(new QTimer(this)), render_requested_(false)
{
  // clang-format off
  connect(update_timer_,
          &QTimer::timeout,
          this,
          &RenderPipeline::onUpdateTimer);
  // clang-format on
  qInfo() << "[RenderPipeline::RenderPipeline] RenderPipeline created";
}

RenderPipeline::~RenderPipeline()
{
  stopUpdate();
  removeAllLayers();
  qInfo() << "[RenderPipeline::RenderPipeline] RenderPipeline destroyed";
}

void RenderPipeline::startUpdate()
{
  if (update_timer_->isActive())
  {
    qWarning() << "[RenderPipeline::startUpdate] Update loop already running";
    return;
  }

  last_update_ = std::chrono::steady_clock::now();
  last_render_ = last_update_;

  update_timer_->start(UPDATE_RATE_MS);

  qInfo() << "[RenderPipeline::startUpdate] Update loop started at" << (1000.0 / UPDATE_RATE_MS) << "Hz";
  emit updateStarted();
}

void RenderPipeline::stopUpdate()
{
  if (update_timer_->isActive())
  {
    update_timer_->stop();
    qInfo() << "[RenderPipeline::stopUpdate] Update loop stopped";
    emit updateStopped();
  }
}

void RenderPipeline::addLayer(const std::string& name, std::shared_ptr<ROBOGait::map::layer::LayerInterface> layer)
{
  if (!layer)
  {
    qWarning() << "[RenderPipeline::addLayer] Null layer pointer";
    return;
  }

  if (layer_map_.find(name) != layer_map_.end())
  {
    qWarning() << "[RenderPipeline::addLayer] Layer already exists:" << QString::fromStdString(name);
    return;
  }

  layer_map_[name] = std::move(layer);
  qInfo() << "[RenderPipeline::addLayer] Added layer:" << QString::fromStdString(name);
}

void RenderPipeline::removeLayer(const std::string& name)
{
  auto it = layer_map_.find(name);
  if (it == layer_map_.end())
  {
    qWarning() << "[RenderPipeline::removeLayer] Layer not found:" << QString::fromStdString(name);
    return;
  }

  layer_map_.erase(it);
  qInfo() << "[RenderPipeline::removeLayer] Removed layer:" << QString::fromStdString(name);
}

void RenderPipeline::removeAllLayers()
{
  layer_map_.clear();
  qInfo() << "[RenderPipeline::removeAllLayers] Cleared all layers";
}

bool RenderPipeline::isRunning() const { return update_timer_ && update_timer_->isActive(); }

void RenderPipeline::onUpdateTimer()
{
  auto now = std::chrono::steady_clock::now();
  last_update_ = now;

  updateLayers();

  bool should_render = false;
  {
    QMutexLocker lock(&render_mutex_);
    should_render = render_requested_;
    render_requested_ = false;
  }

  if (!should_render && anyLayerNeedsRender())
  {
    should_render = true;
  }

  auto time_since_render = std::chrono::duration<double>(now - last_render_).count();

  if (time_since_render > FORCE_RENDER_INTERVAL_SEC)
  {
    should_render = true;
  }

  if (should_render)
  {
    renderFrame();
    clearLayerRenderRequests();
    last_render_ = now;
  }
}

void RenderPipeline::updateLayers()
{
  for (auto& pair : layer_map_)
  {
    if (pair.second)
    {
      pair.second->update();
    }
  }
}

bool RenderPipeline::anyLayerNeedsRender() const
{
  for (const auto& pair : layer_map_)
  {
    if (pair.second && pair.second->needsRender())
    {
      return true;
    }
  }

  return false;
}

void RenderPipeline::clearLayerRenderRequests()
{
  for (auto& pair : layer_map_)
  {
    if (pair.second)
    {
      pair.second->clearRenderRequest();
    }
  }
}

void RenderPipeline::renderFrame()
{
  static auto last_fps = std::chrono::steady_clock::now();
  static int frame_count = 0;
  frame_count++;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration<double>(now - last_fps).count();

  double fps = 0.0;
  if (elapsed >= 1.0)
  {
    fps = frame_count / elapsed;
    frame_count = 0;
    last_fps = now;
  }

  emit frameReady(fps);
}
