#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QPointF>
#include <QRectF>
#include <QtGlobal>

#include "Context/RobotContext.hpp"
#include "Loader/YamlLoader.hpp"
#include "Map/MapVisualizationManager.hpp"
#include "Map/Utils/Utils.hpp"

using namespace ROBOGait::map::manager;

MapVisualizationManager::MapVisualizationManager() :
    parent_node_(nullptr),
    render_scene_(std::make_shared<ROBOGait::map::rendering::RenderScene>()),
    map_layer_(nullptr),
    robot_layer_(nullptr),
    laser_layer_(nullptr),
    particle_layer_(nullptr),
    render_camera_(std::make_shared<ROBOGait::map::rendering::RenderCamera>()),
    map_source_(std::make_shared<ROBOGait::map::source::MapSource>()),
    pose_source_(std::make_shared<ROBOGait::map::source::RobotPoseSource>()),
    laser_source_(std::make_shared<ROBOGait::map::source::LaserSource>()),
    particle_source_(std::make_shared<ROBOGait::map::source::ParticleCloudSource>()),
    selected_robot_namespace_(""),
    use_namespace_discovery_(true),
    is_initialized_(false),
    subscriptions_active_(false),
    map_available_cache_(false),
    robot_pose_available_cache_(false),
    laser_available_cache_(false),
    particle_cloud_available_cache_(false),
    map_resolution_cache_(0.0),
    scale_meters_cache_(0.0),
    scale_pixels_cache_(0),
    robot_size_(0.5),
    follow_robot_(false)
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
  if (laser_source_)
  {
    laser_source_->initialize(parent_node_);
  }
  if (particle_source_)
  {
    particle_source_->initialize(parent_node_);
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
    if (laser_source_)
    {
      laser_source_->setRobotContext(context);
    }
    if (particle_source_)
    {
      particle_source_->setRobotContext(context);
    }
  }

  // Create new displays for this robot
  createLayers();
}

bool MapVisualizationManager::isInitialized() const { return is_initialized_; }

bool MapVisualizationManager::isMapAvailable() const { return map_source_ && map_source_->isAvailable(); }

bool MapVisualizationManager::isRobotPoseAvailable() const { return pose_source_ && pose_source_->isAvailable(); }

bool MapVisualizationManager::isLaserAvailable() const { return laser_source_ && laser_source_->isAvailable(); }

bool MapVisualizationManager::isParticleCloudAvailable() const { return particle_source_ && particle_source_->isAvailable(); }

double MapVisualizationManager::getZoomLevel() const
{
  if (!render_camera_)
  {
    qWarning() << "[MapVisualizationManager::getZoomLevel] Render camera not available";
    return 1.0;
  }

  return render_camera_->getZoom();
}

double MapVisualizationManager::getMapResolution() const { return map_resolution_cache_; }

double MapVisualizationManager::getScaleMeters() const { return scale_meters_cache_; }

int MapVisualizationManager::getScalePixels() const { return scale_pixels_cache_; }

bool MapVisualizationManager::isFollowingRobot() const { return follow_robot_; }

void MapVisualizationManager::setFollowRobot(bool follow_robot)
{

  if (follow_robot_ != follow_robot)
  {

    follow_robot_ = follow_robot;
    emit followRobotChanged();
    updateFollowRobotCamera();
  }
}

bool MapVisualizationManager::screenToMap(const QPointF& screen_point, QPointF& map_point) const
{
  if (!map_layer_ || !render_camera_ || !render_scene_)
  {
    qCritical() << "[MapVisualizationManager::screenToMap] Cannot convert screen to map coordinates because render layers or camera are not available";
    return false;
  }

  const auto map_data = map_layer_->getMapData();
  if (!map_data || !map_data->isAvailable())
  {
    qCritical() << "[MapVisualizationManager::screenToMap] Cannot convert screen to map coordinates because map data is not available";
    return false;
  }

  bool invertible = false;
  const QMatrix4x4 inv_transform = render_camera_->getMatrix().inverted(&invertible);
  if (!invertible)
  {
    qCritical() << "[MapVisualizationManager::screenToMap] Cannot convert screen to map coordinates because camera transform is not invertible";
    return false;
  }

  const QVector4D screen_vec(screen_point.x(), screen_point.y(), 0.0f, 1.0f);
  const QVector4D world_vec = inv_transform * screen_vec;

  map_point = QPointF(world_vec.x(), world_vec.y());
  return true;
}

void MapVisualizationManager::setManualRobotPose(double x, double y, double theta)
{
  if (!pose_source_)
  {
    qCritical() << "[MapVisualizationManager::setManualRobotPose] Pose source not available";
    return;
  }

  auto robot_pose_data = pose_source_->getRobotPoseData();
  if (!robot_pose_data)
  {
    qCritical() << "[MapVisualizationManager::setManualRobotPose] Robot pose data not available";
    return;
  }

  ROBOGait::map::data::RobotPoseData::RobotPoseMetadata metadata;
  metadata.x = x;
  metadata.y = y;
  metadata.theta = theta;
  robot_pose_data->setPose(metadata);

  updateAvailability();

  if (robot_layer_item_)
  {
    robot_layer_item_->update();
  }
}

QVariantMap MapVisualizationManager::getRobotPose() const
{
  QVariantMap pose;
  pose["available"] = false;

  if (!pose_source_)
  {
    qCritical() << "[MapVisualizationManager::getRobotPose] Pose source not available";
    return pose;
  }

  const auto robot_pose_data = pose_source_->getRobotPoseData();
  if (!robot_pose_data || !robot_pose_data->isAvailable())
  {
    return pose;
  }

  const auto metadata = robot_pose_data->getMetadata();
  pose["x"] = metadata.x;
  pose["y"] = metadata.y;
  pose["theta"] = metadata.theta;
  pose["available"] = true;
  return pose;
}

bool MapVisualizationManager::isMapPointInside(double x, double y) const
{
  if (!map_layer_)
  {
    qCritical() << "[MapVisualizationManager::isMapPointInside] Map layer not available";
    return false;
  }

  if (!map_layer_->getMapData())
  {
    qCritical() << "[MapVisualizationManager::isMapPointInside] Map data not available";
    return false;
  }

  const auto map_metadata = map_layer_->getMapData()->getMetadata();
  const double width_m = static_cast<double>(map_metadata.width) * map_metadata.resolution;
  const double height_m = static_cast<double>(map_metadata.height) * map_metadata.resolution;

  if (width_m <= 0.0 || height_m <= 0.0)
  {
    qCritical() << "[MapVisualizationManager::isMapPointInside] Invalid map dimensions";
    return false;
  }

  const double origin_x = map_metadata.origin_x;
  const double origin_y = map_metadata.origin_y;
  const double theta = map_metadata.origin_theta;

  const double cos_t = std::cos(theta);
  const double sin_t = std::sin(theta);

  const double dx = x - origin_x;
  const double dy = y - origin_y;

  const double local_x = cos_t * dx + sin_t * dy;
  const double local_y = -sin_t * dx + cos_t * dy;

  return (local_x >= 0.0 && local_x <= width_m && local_y >= 0.0 && local_y <= height_m);
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
  if (laser_source_)
  {
    laser_source_->start();
  }
  if (particle_source_)
  {
    particle_source_->start();
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
  if (laser_source_)
  {
    laser_source_->stop();
  }
  if (particle_source_)
  {
    particle_source_->stop();
  }

  updateAvailability();

  if (map_layer_item_)
  {
    map_layer_item_->update();
  }

  if (robot_layer_item_)
  {
    robot_layer_item_->update();
  }

  if (particle_layer_item_)
  {
    particle_layer_item_->update();
  }

  if (follow_robot_)
  {
    follow_robot_ = false;
    emit followRobotChanged();
  }

  subscriptions_active_ = false;

  qInfo() << "[MapVisualizationManager::destroySubscriptions] Subscriptions destroyed";
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

  map_layer_item_->setRenderScene(render_scene_);
  map_layer_item_->setRenderer(map_layer_);
  map_layer_item_->setCamera(render_camera_);

  if (robot_layer_item_)
  {
    map_layer_item_->setSyncItem(robot_layer_item_);
  }
  else
  {
    qWarning() << "[MapVisualizationManager::registerMapLayerItem] Robot layer item not registered yet, MapLayerItem will be registered without sync item";
  }

  // clang-format off
  connect(map_layer_item_,
          &ROBOGait::map::item::MapLayerItem::zoomChanged,
          this,
          &MapVisualizationManager::zoomLevelChanged,
          Qt::QueuedConnection);
  // clang-format on

  qInfo() << "[MapVisualizationManager::registerMapLayerItem] Item registered";
}

void MapVisualizationManager::setRobotPoseUpdatesEnabled(bool enabled)
{
  if (!pose_source_)
  {
    return;
  }

  pose_source_->setPaused(!enabled);
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

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(robot_layer_item_);
    map_layer_item_->update();
  }

  qInfo() << "[MapVisualizationManager::registerRobotLayerItem] Item registered";
}

void MapVisualizationManager::registerLaserLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerLaserLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::LaserLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerLaserLayerItem] Invalid item type";
    return;
  }

  laser_layer_item_ = layer_item;

  if (!laser_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerLaserLayerItem] Failed to register LaserLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerLaserLayerItem] Render scene not available, cannot set render scene for LaserLayerItem";
    return;
  }

  if (!laser_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerLaserLayerItem] Laser layer not available, cannot set renderer for LaserLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerLaserLayerItem] Render camera not available, cannot set camera for LaserLayerItem";
    return;
  }

  laser_layer_item_->setRenderScene(render_scene_);
  laser_layer_item_->setRenderer(laser_layer_);
  laser_layer_item_->setCamera(render_camera_);

  qInfo() << "[MapVisualizationManager::registerLaserLayerItem] Item registered";
}

void MapVisualizationManager::registerParticleCloudLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerParticleCloudLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::ParticleCloudLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerParticleCloudLayerItem] Invalid item type";
    return;
  }

  particle_layer_item_ = layer_item;

  if (!particle_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerParticleCloudLayerItem] Failed to register ParticleCloudLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerParticleCloudLayerItem] Render scene not available, cannot set render scene for ParticleCloudLayerItem";
    return;
  }

  if (!particle_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerParticleCloudLayerItem] Particle layer not available, cannot set renderer for ParticleCloudLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerParticleCloudLayerItem] Render camera not available, cannot set camera for ParticleCloudLayerItem";
    return;
  }

  particle_layer_item_->setRenderScene(render_scene_);
  particle_layer_item_->setRenderer(particle_layer_);
  particle_layer_item_->setCamera(render_camera_);

  qInfo() << "[MapVisualizationManager::registerParticleCloudLayerItem] Item registered";
}

void MapVisualizationManager::zoomIn()
{
  if (!render_camera_)
  {
    qWarning() << "[MapVisualizationManager::zoomIn] Render camera not available";
    return;
  }

  setFollowRobot(false);
  render_camera_->zoomByFactor(rendering::RenderCamera::ZOOM_FACTOR);

  emit zoomLevelChanged();

  updateScale();

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

  setFollowRobot(false);
  render_camera_->zoomByFactor(1.0 / rendering::RenderCamera::ZOOM_FACTOR);

  emit zoomLevelChanged();

  updateScale();

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

  setFollowRobot(false);
  const auto metadata = map_layer_->getMapData()->getMetadata();
  const double width_m = static_cast<double>(metadata.width) * metadata.resolution;
  const double height_m = static_cast<double>(metadata.height) * metadata.resolution;

  if (width_m <= 0.0 || height_m <= 0.0)
  {
    qWarning() << "[MapVisualizationManager::fitToView] Invalid map dimensions";
    return;
  }

  const double origin_x = metadata.origin_x;
  const double origin_y = metadata.origin_y;
  const double theta = metadata.origin_theta;
  const double cos_t = std::cos(theta);
  const double sin_t = std::sin(theta);

  auto mapPoint = [&](double x, double y) { return QPointF(origin_x + cos_t * x - sin_t * y, origin_y + sin_t * x + cos_t * y); };

  const QPointF p0 = mapPoint(0.0, 0.0);
  const QPointF p1 = mapPoint(width_m, 0.0);
  const QPointF p2 = mapPoint(0.0, height_m);
  const QPointF p3 = mapPoint(width_m, height_m);

  const double min_x = std::min({p0.x(), p1.x(), p2.x(), p3.x()});
  const double max_x = std::max({p0.x(), p1.x(), p2.x(), p3.x()});
  const double min_y = std::min({p0.y(), p1.y(), p2.y(), p3.y()});
  const double max_y = std::max({p0.y(), p1.y(), p2.y(), p3.y()});

  const QRectF map_rect(QPointF(min_x, min_y), QPointF(max_x, max_y));
  render_camera_->fitToRect(map_rect);

  emit zoomLevelChanged();

  updateScale();

  if (!map_layer_item_ || !robot_layer_item_)
  {
    qWarning() << "[MapVisualizationManager::fitToView] Layer items not registered yet, cannot update view";
    return;
  }

  map_layer_item_->update();
  robot_layer_item_->update();
}

bool MapVisualizationManager::generateMapPreview(const QString& map_name)
{
  if (map_name.isEmpty())
  {
    qWarning() << "[MapVisualizationManager::generateMapPreview] Empty map name provided";
    return false;
  }

  if (!map_layer_)
  {
    qCritical() << "[MapVisualizationManager::generateMapPreview] Map layer not available";
    return false;
  }

  if (!map_layer_->getMapData())
  {
    qCritical() << "[MapVisualizationManager::generateMapPreview] Map data not available";
    return false;
  }

  if (!ROBOGait::map::utils::generateMapPreview(*map_layer_->getMapData(), map_name))
  {
    qCritical() << "[MapVisualizationManager::generateMapPreview] Failed to generate map preview for map:" << map_name;
    return false;
  }

  return true;
}

bool MapVisualizationManager::deleteMapPreview(const QString& map_name)
{
  if (map_name.isEmpty())
  {
    qWarning() << "[MapVisualizationManager::deleteMapPreview] Empty map name provided";
    return false;
  }

  return ROBOGait::map::utils::deleteMapPreview(map_name);
}

QString MapVisualizationManager::getMapPreviewPath(const QString& map_name)
{
  if (map_name.isEmpty())
  {
    qCritical() << "[MapVisualizationManager::getMapPreviewPath] Map name is empty";
    return QString();
  }

  return ROBOGait::map::utils::getMapPreviewPath(map_name);
}

void MapVisualizationManager::clearMap()
{
  if (map_source_)
  {
    const auto map_data = map_source_->getMapData();
    if (map_data)
    {
      map_data->reset();
    }
  }

  if (map_layer_)
  {
    map_layer_->update();
  }

  if (map_layer_item_)
  {
    map_layer_item_->update();
  }

  updateAvailability();
}

void MapVisualizationManager::resetParticleCloud()
{
  if (particle_source_)
  {
    const auto cloud_data = particle_source_->getParticleCloudData();
    if (cloud_data)
    {
      cloud_data->reset();
    }
  }

  if (particle_layer_)
  {
    particle_layer_->update();
  }

  if (particle_layer_item_)
  {
    particle_layer_item_->update();
  }

  updateAvailability();
}

void MapVisualizationManager::onFrameReady()
{
  updateFollowRobotCamera();
  updateAvailability();
}

void MapVisualizationManager::createLayers()
{
  if (!is_initialized_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Not initialized";
    return;
  }

  if (!map_source_ || !pose_source_ || !laser_source_ || !particle_source_)
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
  const auto laser_data = laser_source_->getLaserScanData();
  const auto particle_data = particle_source_->getParticleCloudData();

  map_layer_ = std::make_shared<ROBOGait::map::layer::MapLayer>();
  map_layer_->setMapData(map_data);

  robot_layer_ = std::make_shared<ROBOGait::map::layer::RobotLayer>();
  robot_layer_->setRobotPoseData(pose_data);
  robot_layer_->setRobotSize(robot_size_);

  laser_layer_ = std::make_shared<ROBOGait::map::layer::LaserLayer>();
  laser_layer_->setLaserScanData(laser_data);

  particle_layer_ = std::make_shared<ROBOGait::map::layer::ParticleCloudLayer>();
  particle_layer_->setParticleCloudData(particle_data);

  if (!map_layer_ || !robot_layer_ || !laser_layer_ || !particle_layer_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Failed to create render layers";
    return;
  }

  render_scene_->setMapLayer(map_layer_);
  render_scene_->setRobotLayer(robot_layer_);
  render_scene_->setLaserLayer(laser_layer_);
  render_scene_->setParticleCloudLayer(particle_layer_);

  updateAvailability();

  qInfo() << "[MapVisualizationManager::createLayers] Layers created";
}

void MapVisualizationManager::destroyLayers()
{
  if (!map_layer_ && !robot_layer_ && !laser_layer_ && !particle_layer_)
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
  render_scene_->setLaserLayer(nullptr);
  render_scene_->setParticleCloudLayer(nullptr);

  if (map_layer_item_)
  {
    map_layer_item_->setRenderer(nullptr);
  }
  if (robot_layer_item_)
  {
    robot_layer_item_->setRenderer(nullptr);
  }
  if (laser_layer_item_)
  {
    laser_layer_item_->setRenderer(nullptr);
  }
  if (particle_layer_item_)
  {
    particle_layer_item_->setRenderer(nullptr);
  }

  map_layer_.reset();
  robot_layer_.reset();
  laser_layer_.reset();
  particle_layer_.reset();
  map_available_cache_ = false;
  robot_pose_available_cache_ = false;
  laser_available_cache_ = false;
  particle_cloud_available_cache_ = false;
  map_resolution_cache_ = 0.0;
  scale_meters_cache_ = 0.0;
  scale_pixels_cache_ = 0;

  emit mapAvailableChanged();
  emit robotPoseAvailableChanged();
  emit laserAvailableChanged();
  emit particleCloudAvailableChanged();
  emit mapResolutionChanged();
  emit scaleChanged();

  qInfo() << "[MapVisualizationManager::destroyLayers] Layers destroyed";
}

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

  const bool laser_available = isLaserAvailable();
  if (laser_available != laser_available_cache_)
  {
    laser_available_cache_ = laser_available;
    emit laserAvailableChanged();
  }

  const bool particle_available = isParticleCloudAvailable();
  if (particle_available != particle_cloud_available_cache_)
  {
    particle_cloud_available_cache_ = particle_available;
    emit particleCloudAvailableChanged();
  }

  updateMapResolution();
  updateScale();
}

void MapVisualizationManager::updateMapResolution()
{
  double new_resolution = 0.0;

  if (map_source_ && map_source_->isAvailable())
  {
    const auto map_data = map_source_->getMapData();
    if (map_data)
    {
      new_resolution = map_data->getMetadata().resolution;
    }
  }

  if (!qFuzzyCompare(new_resolution + 1.0, map_resolution_cache_ + 1.0))
  {
    map_resolution_cache_ = new_resolution;
    emit mapResolutionChanged();
  }
}

void MapVisualizationManager::updateScale()
{
  double new_meters = 0.0;
  int new_pixels = 0;

  if (map_available_cache_ && render_camera_)
  {
    const double ppm = render_camera_->getZoom();
    if (ppm > 0.0)
    {
      const int target_px = rendering::RenderCamera::SCALE_BAR_TARGET_LENGTH_PX;
      new_pixels = target_px;
      new_meters = static_cast<double>(target_px) / ppm;
    }
  }

  if (!qFuzzyCompare(new_meters + 1.0, scale_meters_cache_ + 1.0) || new_pixels != scale_pixels_cache_)
  {
    scale_meters_cache_ = new_meters;
    scale_pixels_cache_ = new_pixels;
    emit scaleChanged();
  }
}

void MapVisualizationManager::updateFollowRobotCamera()
{
  if (!follow_robot_)
  {
    return;
  }

  if (!robot_layer_ || !render_camera_)
  {
    qWarning() << "[MapVisualizationManager::updateFollowRobotCamera] Follow enabled but camera or robot layer not available";
    return;
  }

  const auto pose = robot_layer_->getInterpolatedPose();
  render_camera_->setViewCenter(QPointF(pose.x, pose.y));
  if (map_layer_item_)
  {
    map_layer_item_->update();
  }
  if (robot_layer_item_)
  {
    robot_layer_item_->update();
  }
}
