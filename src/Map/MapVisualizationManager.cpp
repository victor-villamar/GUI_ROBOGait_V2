#include <QDebug>
#include <QRectF>

#include "Context/RobotContext.hpp"
#include "Loader/YamlLoader.hpp"
#include "Map/MapVisualizationManager.hpp"

using namespace ROBOGait::map::manager;

MapVisualizationManager::MapVisualizationManager() :
    parent_node_(nullptr),
    render_scene_(std::make_shared<ROBOGait::map::rendering::RenderScene>()),
    map_layer_(nullptr),
    robot_layer_(nullptr),
    render_camera_(std::make_shared<ROBOGait::map::rendering::RenderCamera>()),
    map_source_(std::make_shared<ROBOGait::map::source::MapSource>()),
    pose_source_(std::make_shared<ROBOGait::map::source::RobotPoseSource>()),
    selected_robot_namespace_(""),
    use_namespace_discovery_(true),
    is_initialized_(false),
    subscriptions_active_(false),
    map_available_cache_(false),
    robot_pose_available_cache_(false),
    robot_size_(0.5)
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (yaml_loader.isLoaded())
  {
    const double configured_size = yaml_loader.getValue<double>("map.robot_size", robot_size_);

    if (configured_size > 0.0)
    {
      robot_size_ = configured_size;
    }
    else
    {
      qWarning() << "[MapVisualizationManager::MapVisualizationManager] Invalid map.robot_size (must be > 0), using default:" << robot_size_;
    }
  }
  else
  {
    qWarning() << "[MapVisualizationManager::MapVisualizationManager] YAML not loaded, using default robot_size:" << robot_size_;
  }

  if (render_scene_ && render_scene_->getPipeline())
  {
    // clang-format off
    connect(render_scene_->getPipeline().get(),
            &ROBOGait::map::rendering::RenderPipeline::frameReady,
            this,
            &MapVisualizationManager::onFrameReady);
    // clang-format on
  }

  qInfo() << "[MapVisualizationManager::MapVisualizationManager] Map Visualization Manager created";
}

MapVisualizationManager::~MapVisualizationManager()
{
  destroySubscriptions();
  destroyLayers();

  qInfo() << "[MapVisualizationManager] Manager destroyed";
}

void MapVisualizationManager::setROSNode(rclcpp::Node* parent_node)
{
  if (is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::setROSNode] Already initialized";
    return;
  }

  if (!parent_node)
  {
    qCritical() << "[MapVisualizationManager::setROSNode] Null node pointer";
    return;
  }

  parent_node_ = parent_node;

  if (map_source_)
  {
    map_source_->initialize(parent_node_);
  }
  if (pose_source_)
  {
    pose_source_->initialize(parent_node_);
  }

  is_initialized_ = true;

  emit isInitializedChanged();
}

void MapVisualizationManager::setSelectedRobot(const QString& robot_identifier, bool is_namespace)
{
  if (!is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::setSelectedRobot] Not initialized";
    return;
  }

  // Destroy previous displays if they exist
  if (subscriptions_active_)
  {
    destroySubscriptions();
  }

  destroyLayers();

  // Store new robot selection
  selected_robot_namespace_ = robot_identifier;
  use_namespace_discovery_ = is_namespace;

  // Create robot context
  ROBOGait::context::RobotContext context;
  if (context.setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_))
  {
    if (map_source_)
    {
      map_source_->setRobotContext(context);
    }
    if (pose_source_)
    {
      pose_source_->setRobotContext(context);
    }
  }

  // Create new displays for this robot
  createLayers();
}

void MapVisualizationManager::activateSubscriptions()
{
  if (!is_initialized_)
  {
    qWarning() << "[MapVisualizationManager::activateSubscriptions] Not initialized";
    return;
  }

  if (subscriptions_active_)
  {
    qWarning() << "[MapVisualizationManager::activateSubscriptions] Already active";
    return;
  }

  if (!map_layer_ || !robot_layer_)
  {
    qWarning() << "[MapVisualizationManager::activateSubscriptions] Render layers not created yet";
    return;
  }

  if (map_source_)
  {
    map_source_->start();
  }
  if (pose_source_)
  {
    pose_source_->start();
  }
  if (render_scene_)
  {
    render_scene_->start();
  }

  subscriptions_active_ = true;
  updateAvailability();
}

void MapVisualizationManager::destroySubscriptions()
{
  if (!subscriptions_active_)
  {
    return;
  }

  if (render_scene_)
  {
    render_scene_->stop();
  }
  if (map_source_)
  {
    map_source_->stop();
  }
  if (pose_source_)
  {
    pose_source_->stop();
  }

  subscriptions_active_ = false;

  qInfo() << "[MapVisualizationManager::destroySubscriptions] Subscriptions destroyed";
}

bool MapVisualizationManager::isInitialized() const { return is_initialized_; }

bool MapVisualizationManager::isMapAvailable() const { return map_source_ && map_source_->isAvailable(); }

bool MapVisualizationManager::isRobotPoseAvailable() const { return pose_source_ && pose_source_->isAvailable(); }

double MapVisualizationManager::getZoomLevel() const
{
  if (!render_camera_)
  {
    qWarning() << "[MapVisualizationManager::getZoomLevel] Render camera not available";
    return 1.0;
  }

  return render_camera_->getZoom();
}

void MapVisualizationManager::createLayers()
{
  if (!is_initialized_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Not initialized";
    return;
  }

  if (!map_source_ || !pose_source_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Data sources not initialized";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Render scene not available";
    return;
  }

  const auto map_data = map_source_->getMapData();
  const auto pose_data = pose_source_->getRobotPoseData();

  map_layer_ = std::make_shared<ROBOGait::map::layer::MapLayer>(map_data);

  robot_layer_ = std::make_shared<ROBOGait::map::layer::RobotLayer>(pose_data);
  robot_layer_->setRobotSize(robot_size_);

  if (!map_layer_ || !robot_layer_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Failed to create render layers";
    return;
  }

  render_scene_->setMapLayer(map_layer_);
  render_scene_->setRobotLayer(robot_layer_);

  updateAvailability();

  qInfo() << "[MapVisualizationManager::createLayers] Layers created";
}

void MapVisualizationManager::destroyLayers()
{
  if (!map_layer_ && !robot_layer_)
  {
    qWarning() << "[MapVisualizationManager::destroyLayers] No layers to destroy";
    return;
  }

  if (!render_scene_)
  {
    qWarning() << "[MapVisualizationManager::destroyLayers] Render scene not available, cannot properly disconnect layers from scene";
    return;
  }

  render_scene_->stop();
  render_scene_->setMapLayer(nullptr);
  render_scene_->setRobotLayer(nullptr);

  if (map_layer_item_)
  {
    map_layer_item_->setRenderer(nullptr);
  }
  if (robot_layer_item_)
  {
    robot_layer_item_->setRenderer(nullptr);
  }

  map_layer_.reset();
  robot_layer_.reset();
  map_available_cache_ = false;
  robot_pose_available_cache_ = false;

  emit mapAvailableChanged();
  emit robotPoseAvailableChanged();

  qInfo() << "[MapVisualizationManager::destroyLayers] Layers destroyed";
}

void MapVisualizationManager::registerMapLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerMapLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::MapLayerItem*>(item);

  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerMapLayerItem] Invalid item type";
    return;
  }

  map_layer_item_ = layer_item;

  if (!map_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerMapLayerItem] Failed to register MapLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerMapLayerItem] Render scene not available, cannot set render scene for MapLayerItem";
    return;
  }

  if (!map_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerMapLayerItem] Map layer not available, cannot set renderer for MapLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerMapLayerItem] Render camera not available, cannot set camera for MapLayerItem";
    return;
  }

  if (!robot_layer_item_)
  {
    qWarning() << "[MapVisualizationManager::registerMapLayerItem] Robot layer item not registered yet, MapLayerItem will be registered without sync item "
                  "until RobotLayerItem is registered";
    return;
  }

  map_layer_item_->setRenderScene(render_scene_);
  map_layer_item_->setRenderer(map_layer_);
  map_layer_item_->setCamera(render_camera_);
  map_layer_item_->setSyncItem(robot_layer_item_);

  // clang-format off
  connect(map_layer_item_,
          &ROBOGait::map::item::MapLayerItem::zoomChanged,
          this,
          &MapVisualizationManager::zoomLevelChanged,
          Qt::QueuedConnection);
  // clang-format on

  qInfo() << "[MapVisualizationManager::registerMapLayerItem] Item registered";
}

void MapVisualizationManager::registerRobotLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerRobotLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::RobotLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerRobotLayerItem] Invalid item type";
    return;
  }

  robot_layer_item_ = layer_item;

  if (!robot_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerRobotLayerItem] Failed to register RobotLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerRobotLayerItem] Render scene not available, cannot set render scene for RobotLayerItem";
    return;
  }

  if (!robot_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerRobotLayerItem] Robot layer not available, cannot set renderer for RobotLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerRobotLayerItem] Render camera not available, cannot set camera for RobotLayerItem";
    return;
  }

  robot_layer_item_->setRenderScene(render_scene_);
  robot_layer_item_->setRenderer(robot_layer_);
  robot_layer_item_->setCamera(render_camera_);

  qInfo() << "[MapVisualizationManager::registerRobotLayerItem] Item registered";
}

void MapVisualizationManager::zoomIn()
{
  if (!render_camera_)
  {
    qWarning() << "[MapVisualizationManager::zoomIn] Render camera not available";
    return;
  }

  render_camera_->zoomByFactor(1.1);

  emit zoomLevelChanged();

  if (map_layer_item_)
  {
    map_layer_item_->update();
  }
  if (robot_layer_item_)
  {
    robot_layer_item_->update();
  }
}

void MapVisualizationManager::zoomOut()
{
  if (!render_camera_)
  {
    qWarning() << "[MapVisualizationManager::zoomOut] Render camera not available";
    return;
  }

  render_camera_->zoomByFactor(1.0 / 1.1);

  emit zoomLevelChanged();
  if (map_layer_item_)
  {
    map_layer_item_->update();
  }
  if (robot_layer_item_)
  {
    robot_layer_item_->update();
  }
}

void MapVisualizationManager::fitToView()
{
  if (!render_camera_ || !map_layer_ || !map_layer_->getMapData())
  {
    qWarning() << "[MapVisualizationManager::fitToView] Required components not available";
    return;
  }

  const auto metadata = map_layer_->getMapData()->getMetadata();
  const double width_m = static_cast<double>(metadata.width) * metadata.resolution;
  const double height_m = static_cast<double>(metadata.height) * metadata.resolution;

  if (width_m <= 0.0 || height_m <= 0.0)
  {
    qWarning() << "[MapVisualizationManager::fitToView] Invalid map dimensions";
    return;
  }

  const double origin_x = metadata.origin_x;
  const double origin_y = -(metadata.origin_y + height_m);

  const QRectF map_rect(origin_x, origin_y, width_m, height_m);
  render_camera_->fitToRect(map_rect);

  emit zoomLevelChanged();
  if (!map_layer_item_ || !robot_layer_item_)
  {
    qWarning() << "[MapVisualizationManager::fitToView] Layer items not registered yet, cannot update view";
    return;
  }

  map_layer_item_->update();
  robot_layer_item_->update();
}

void MapVisualizationManager::onFrameReady() { updateAvailability(); }

void MapVisualizationManager::updateAvailability()
{
  const bool map_available = isMapAvailable();

  if (map_available != map_available_cache_)
  {
    map_available_cache_ = map_available;
    emit mapAvailableChanged();
  }

  const bool robot_available = isRobotPoseAvailable();

  if (robot_available != robot_pose_available_cache_)
  {
    robot_pose_available_cache_ = robot_available;
    emit robotPoseAvailableChanged();
  }
}
