#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

#include <QDebug>
#include <QPointF>
#include <QRectF>
#include <QVector4D>
#include <QtGlobal>

#include "Context/RobotContext.hpp"
#include "Loader/YamlLoader.hpp"
#include "Map/MapVisualizationManager.hpp"
#include "Map/Utils/Utils.hpp"

using namespace ROBOGait::map::manager;

namespace
{
using SplinePathEditor = ROBOGait::map::interaction::SplinePathEditor;

constexpr double MANUAL_LIVE_PATH_FINISH_DISTANCE_M = 0.20;

SplinePathEditor::SmoothingTuningPx loadSmoothingTuningPx(const ROBOGait::loader::YamlLoader& yaml_loader)
{
  SplinePathEditor::SmoothingTuningPx tuning;

  if (!yaml_loader.isLoaded())
  {
    return tuning;
  }

  tuning.resample_spacing_px = yaml_loader.getValue<double>("map.spline_path.resample_spacing_px", tuning.resample_spacing_px);
  tuning.smoothing_window_radius = yaml_loader.getValue<int>("map.spline_path.smoothing_window_radius", tuning.smoothing_window_radius);
  tuning.catmull_alpha = yaml_loader.getValue<double>("map.spline_path.catmull_alpha", tuning.catmull_alpha);
  tuning.sample_spacing_px = yaml_loader.getValue<double>("map.spline_path.sample_spacing_px", tuning.sample_spacing_px);
  return tuning;
}

SplinePathEditor::EditReductionTuningPx loadEditReductionTuningPx(const ROBOGait::loader::YamlLoader& yaml_loader)
{
  SplinePathEditor::EditReductionTuningPx tuning;

  if (!yaml_loader.isLoaded())
  {
    return tuning;
  }

  tuning.simplify_tolerance_px = yaml_loader.getValue<double>("map.spline_path.simplify_tolerance_px", tuning.simplify_tolerance_px);
  tuning.max_anchor_points = yaml_loader.getValue<int>("map.spline_path.max_anchor_points", tuning.max_anchor_points);
  return tuning;
}

} // namespace

MapVisualizationManager::MapVisualizationManager() :
    parent_node_(nullptr),
    render_scene_(std::make_shared<ROBOGait::map::rendering::RenderScene>()),
    render_camera_(std::make_shared<ROBOGait::map::rendering::RenderCamera>()),
    map_layer_(nullptr),
    robot_layer_(nullptr),
    goal_robot_layer_(nullptr),
    path_layer_(nullptr),
    manual_draw_path_layer_(nullptr),
    live_path_layer_(nullptr),
    laser_layer_(nullptr),
    particle_layer_(nullptr),
    map_source_(std::make_shared<ROBOGait::map::source::MapSource>()),
    pose_source_(std::make_shared<ROBOGait::map::source::RobotPoseSource>()),
    goal_robot_pose_data_(nullptr),
    manual_path_data_(nullptr),
    manual_draw_path_data_(nullptr),
    path_source_(std::make_shared<ROBOGait::map::source::PathSource>()),
    laser_source_(std::make_shared<ROBOGait::map::source::LaserSource>()),
    particle_source_(std::make_shared<ROBOGait::map::source::ParticleCloudSource>()),
    spline_path_editor_(std::make_unique<ROBOGait::map::interaction::SplinePathEditor>()),
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
    follow_robot_(false),
    manual_live_path_progress_index_(0U),
    manual_live_path_last_pose_stamp_(0U),
    manual_live_path_enabled_(false)
{
  if (spline_path_editor_)
  {
    spline_path_editor_->setMapVisualizationManager(this);
  }

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

  if (spline_path_editor_)
  {
    spline_path_editor_->setSmoothingTuningPx(loadSmoothingTuningPx(yaml_loader));
    spline_path_editor_->setEditReductionTuningPx(loadEditReductionTuningPx(yaml_loader));
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
  if (path_source_)
  {
    path_source_->initialize(parent_node_);
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
    if (path_source_)
    {
      path_source_->setRobotContext(context);
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

  const QVector4D screen_vec(static_cast<float>(screen_point.x()), static_cast<float>(screen_point.y()), 0.0f, 1.0f);
  const QVector4D world_vec = inv_transform * screen_vec;

  map_point = QPointF(static_cast<qreal>(world_vec.x()), static_cast<qreal>(world_vec.y()));
  return true;
}

bool MapVisualizationManager::mapToScreen(const QPointF& map_point, QPointF& screen_point) const
{
  if (!map_layer_ || !render_camera_ || !render_scene_)
  {
    qCritical() << "[MapVisualizationManager::mapToScreen] Cannot convert map to screen coordinates because render layers or camera are not available";
    return false;
  }

  const auto map_data = map_layer_->getMapData();
  if (!map_data || !map_data->isAvailable())
  {
    qCritical() << "[MapVisualizationManager::mapToScreen] Cannot convert map to screen coordinates because map data is not available";
    return false;
  }

  const QVector4D map_vec(static_cast<float>(map_point.x()), static_cast<float>(map_point.y()), 0.0f, 1.0f);
  const QVector4D screen_vec = render_camera_->getMatrix() * map_vec;

  screen_point = QPointF(static_cast<qreal>(screen_vec.x()), static_cast<qreal>(screen_vec.y()));
  return true;
}

QVariantMap MapVisualizationManager::screenToMap(double screen_x, double screen_y) const
{
  QVariantMap result;
  result["available"] = false;

  QPointF map_point;
  if (!screenToMap(QPointF(screen_x, screen_y), map_point))
  {
    return result;
  }

  result["available"] = true;
  result["x"] = map_point.x();
  result["y"] = map_point.y();
  return result;
}

QVariantMap MapVisualizationManager::mapToScreen(double map_x, double map_y) const
{
  QVariantMap result;
  result["available"] = false;

  QPointF screen_point;
  if (!mapToScreen(QPointF(map_x, map_y), screen_point))
  {
    return result;
  }

  result["available"] = true;
  result["x"] = screen_point.x();
  result["y"] = screen_point.y();
  return result;
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
  metadata.x_ = x;
  metadata.y_ = y;
  metadata.theta_ = theta;
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
  pose["x"] = metadata.x_;
  pose["y"] = metadata.y_;
  pose["theta"] = metadata.theta_;
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
  const double width_m = static_cast<double>(map_metadata.width_) * map_metadata.resolution_;
  const double height_m = static_cast<double>(map_metadata.height_) * map_metadata.resolution_;

  if (width_m <= 0.0 || height_m <= 0.0)
  {
    qCritical() << "[MapVisualizationManager::isMapPointInside] Invalid map dimensions";
    return false;
  }

  const double origin_x = map_metadata.origin_x_;
  const double origin_y = map_metadata.origin_y_;
  const double theta = map_metadata.origin_theta_;

  const double cos_t = std::cos(theta);
  const double sin_t = std::sin(theta);

  const double dx = x - origin_x;
  const double dy = y - origin_y;

  const double local_x = cos_t * dx + sin_t * dy;
  const double local_y = -sin_t * dx + cos_t * dy;

  return (local_x >= 0.0 && local_x <= width_m && local_y >= 0.0 && local_y <= height_m);
}

ROBOGait::map::interaction::SplinePathEditor* MapVisualizationManager::getSplinePathEditor() const { return spline_path_editor_.get(); }

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
  if (path_source_)
  {
    path_source_->stop();
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

  if (path_layer_item_)
  {
    path_layer_item_->update();
  }

  if (manual_draw_path_layer_item_)
  {
    manual_draw_path_layer_item_->update();
  }

  if (live_path_layer_item_)
  {
    live_path_layer_item_->update();
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

  bool has_sync_overlay = false;
  if (robot_layer_item_)
  {
    map_layer_item_->setSyncItem(robot_layer_item_);
    has_sync_overlay = true;
  }

  if (goal_robot_layer_item_)
  {
    map_layer_item_->setSyncItem(goal_robot_layer_item_);
    has_sync_overlay = true;
  }

  if (path_layer_item_)
  {
    map_layer_item_->setSyncItem(path_layer_item_);
    has_sync_overlay = true;
  }

  if (manual_draw_path_layer_item_)
  {
    map_layer_item_->setSyncItem(manual_draw_path_layer_item_);
    has_sync_overlay = true;
  }

  if (live_path_layer_item_)
  {
    map_layer_item_->setSyncItem(live_path_layer_item_);
    has_sync_overlay = true;
  }

  if (laser_layer_item_)
  {
    map_layer_item_->setSyncItem(laser_layer_item_);
    has_sync_overlay = true;
  }

  if (particle_layer_item_)
  {
    map_layer_item_->setSyncItem(particle_layer_item_);
    has_sync_overlay = true;
  }

  if (!has_sync_overlay)
  {
    qWarning() << "[MapVisualizationManager::registerMapLayerItem] Overlay items not registered yet, MapLayerItem will be registered without sync overlays";
  }

  // clang-format off
  connect(map_layer_item_,
          &ROBOGait::map::item::MapLayerItem::zoomChanged,
          this,
          &MapVisualizationManager::zoomLevelChanged,
          Qt::QueuedConnection);

  connect(map_layer_item_,
          &ROBOGait::map::item::MapLayerItem::viewTransformChanged,
          this,
          &MapVisualizationManager::viewTransformChanged,
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

void MapVisualizationManager::setPathUpdatesEnabled(bool enabled)
{
  if (!path_source_)
  {
    return;
  }

  manual_live_path_enabled_ = false;
  manual_live_path_points_.clear();
  manual_live_path_progress_index_ = 0U;
  manual_live_path_last_pose_stamp_ = 0U;

  if (enabled)
  {
    path_source_->start();
  }
  else
  {
    path_source_->stop();

    // Ensure the live-path layer repaints immediately after reset.
    if (live_path_layer_)
    {
      live_path_layer_->update();
    }
    if (live_path_layer_item_)
    {
      live_path_layer_item_->update();
    }
  }
}

std::vector<ROBOGait::map::data::PathData::PathPoint> MapVisualizationManager::pathPointsFromVariantList(const QVariantList& points) const
{
  std::vector<ROBOGait::map::data::PathData::PathPoint> path_points;
  path_points.reserve(static_cast<size_t>(points.size()));

  for (const auto& value : points)
  {
    if (!value.canConvert<QVariantMap>())
    {
      continue;
    }

    const QVariantMap map = value.toMap();
    bool ok_x = false;
    bool ok_y = false;
    const double x = map.value("x").toDouble(&ok_x);
    const double y = map.value("y").toDouble(&ok_y);

    if (!ok_x || !ok_y)
    {
      continue;
    }

    path_points.emplace_back(x, y);
  }

  return path_points;
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

void MapVisualizationManager::registerGoalRobotLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerGoalRobotLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::RobotLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerGoalRobotLayerItem] Invalid item type";
    return;
  }

  goal_robot_layer_item_ = layer_item;

  if (!goal_robot_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerGoalRobotLayerItem] Failed to register RobotLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerGoalRobotLayerItem] Render scene not available, cannot set render scene for RobotLayerItem";
    return;
  }

  if (!goal_robot_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerGoalRobotLayerItem] Goal robot layer not available, cannot set renderer for RobotLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerGoalRobotLayerItem] Render camera not available, cannot set camera for RobotLayerItem";
    return;
  }

  goal_robot_layer_item_->setRenderScene(render_scene_);
  goal_robot_layer_item_->setRenderer(goal_robot_layer_);
  goal_robot_layer_item_->setCamera(render_camera_);

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(goal_robot_layer_item_);
  }

  qInfo() << "[MapVisualizationManager::registerGoalRobotLayerItem] Item registered";
}

void MapVisualizationManager::registerPathLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerPathLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::PathLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerPathLayerItem] Invalid item type";
    return;
  }

  path_layer_item_ = layer_item;

  if (!path_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerPathLayerItem] Failed to register PathLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerPathLayerItem] Render scene not available, cannot set render scene for PathLayerItem";
    return;
  }

  if (!path_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerPathLayerItem] Path layer not available, cannot set renderer for PathLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerPathLayerItem] Render camera not available, cannot set camera for PathLayerItem";
    return;
  }

  path_layer_item_->setRenderScene(render_scene_);
  path_layer_item_->setRenderer(path_layer_);
  path_layer_item_->setCamera(render_camera_);

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(path_layer_item_);
  }

  qInfo() << "[MapVisualizationManager::registerPathLayerItem] Item registered";
}

void MapVisualizationManager::registerManualDrawPathLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::PathLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Invalid item type";
    return;
  }

  manual_draw_path_layer_item_ = layer_item;

  if (!manual_draw_path_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Failed to register PathLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Render scene not available, cannot set render scene for PathLayerItem";
    return;
  }

  if (!manual_draw_path_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Manual draw path layer not available, cannot set renderer for PathLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Render camera not available, cannot set camera for PathLayerItem";
    return;
  }

  manual_draw_path_layer_item_->setRenderScene(render_scene_);
  manual_draw_path_layer_item_->setRenderer(manual_draw_path_layer_);
  manual_draw_path_layer_item_->setCamera(render_camera_);

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(manual_draw_path_layer_item_);
  }

  qInfo() << "[MapVisualizationManager::registerManualDrawPathLayerItem] Item registered";
}

void MapVisualizationManager::registerLivePathLayerItem(QObject* item)
{
  if (!item)
  {
    qCritical() << "[MapVisualizationManager::registerLivePathLayerItem] Null item";
    return;
  }

  auto* layer_item = qobject_cast<ROBOGait::map::item::PathLayerItem*>(item);
  if (!layer_item)
  {
    qCritical() << "[MapVisualizationManager::registerLivePathLayerItem] Invalid item type";
    return;
  }

  live_path_layer_item_ = layer_item;

  if (!live_path_layer_item_)
  {
    qCritical() << "[MapVisualizationManager::registerLivePathLayerItem] Failed to register PathLayerItem";
    return;
  }

  if (!render_scene_)
  {
    qCritical() << "[MapVisualizationManager::registerLivePathLayerItem] Render scene not available, cannot set render scene for PathLayerItem";
    return;
  }

  if (!live_path_layer_)
  {
    qCritical() << "[MapVisualizationManager::registerLivePathLayerItem] Live path layer not available, cannot set renderer for PathLayerItem";
    return;
  }

  if (!render_camera_)
  {
    qCritical() << "[MapVisualizationManager::registerLivePathLayerItem] Render camera not available, cannot set camera for PathLayerItem";
    return;
  }

  live_path_layer_item_->setRenderScene(render_scene_);
  live_path_layer_item_->setRenderer(live_path_layer_);
  live_path_layer_item_->setCamera(render_camera_);

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(live_path_layer_item_);
  }

  qInfo() << "[MapVisualizationManager::registerLivePathLayerItem] Item registered";
}

void MapVisualizationManager::setGoalRobotPose(double x, double y, double theta)
{
  if (!goal_robot_pose_data_ || !goal_robot_layer_)
  {
    qCritical() << "[MapVisualizationManager::setGoalRobotPose] Goal robot data not available";
    return;
  }

  ROBOGait::map::data::RobotPoseData::RobotPoseMetadata metadata;
  metadata.x_ = x;
  metadata.y_ = y;
  metadata.theta_ = theta;
  goal_robot_pose_data_->setPose(metadata);

  goal_robot_layer_->update();
  if (goal_robot_layer_item_)
  {
    goal_robot_layer_item_->update();
  }
}

void MapVisualizationManager::clearGoalRobotPose()
{
  if (!goal_robot_pose_data_)
  {
    return;
  }
  if (!goal_robot_pose_data_->isAvailable())
  {
    return;
  }

  goal_robot_pose_data_->reset();
  if (goal_robot_layer_)
  {
    goal_robot_layer_->resetInterpolation();
  }
  if (goal_robot_layer_item_)
  {
    goal_robot_layer_item_->update();
  }
}

void MapVisualizationManager::setManualPathPoints(const QVariantList& points)
{
  ROBOGait::map::data::PathData::PathMetadata metadata;
  metadata.points = pathPointsFromVariantList(points);

  if (manual_path_data_)
  {
    if (metadata.points.empty())
    {
      manual_path_data_->reset();
    }
    else
    {
      manual_path_data_->setPath(metadata);
    }
  }

  if (path_layer_)
  {
    path_layer_->update();
  }
  if (path_layer_item_)
  {
    path_layer_item_->update();
  }
}

void MapVisualizationManager::clearManualPath()
{
  if (manual_path_data_)
  {
    manual_path_data_->reset();
  }

  if (path_layer_)
  {
    path_layer_->update();
  }
  if (path_layer_item_)
  {
    path_layer_item_->update();
  }
}

void MapVisualizationManager::setManualDrawPathPoints(const QVariantList& points)
{
  ROBOGait::map::data::PathData::PathMetadata metadata;
  metadata.points = pathPointsFromVariantList(points);

  if (manual_draw_path_data_)
  {
    if (metadata.points.empty())
    {
      manual_draw_path_data_->reset();
    }

    else
    {
      manual_draw_path_data_->setPath(metadata);
    }
  }

  if (manual_draw_path_layer_)
  {
    manual_draw_path_layer_->update();
  }

  if (manual_draw_path_layer_item_)
  {
    manual_draw_path_layer_item_->update();
  }
}

void MapVisualizationManager::clearManualDrawPath()
{
  if (manual_draw_path_data_)
  {
    manual_draw_path_data_->reset();
  }

  if (manual_draw_path_layer_)
  {
    manual_draw_path_layer_->update();
  }

  if (manual_draw_path_layer_item_)
  {
    manual_draw_path_layer_item_->update();
  }

  if (spline_path_editor_)
  {
    spline_path_editor_->clearCachedPath();
  }
}

void MapVisualizationManager::clearManualDrawPathVisualization()
{
  if (manual_draw_path_data_)
  {
    manual_draw_path_data_->reset();
  }

  if (manual_draw_path_layer_)
  {
    manual_draw_path_layer_->update();
  }

  if (manual_draw_path_layer_item_)
  {
    manual_draw_path_layer_item_->update();
  }
}

void MapVisualizationManager::startManualLivePath(const QVariantList& points)
{
  if (path_source_ && path_source_->isActive())
  {
    path_source_->stop();
  }

  manual_live_path_points_ = pathPointsFromVariantList(points);
  manual_live_path_progress_index_ = 0U;
  manual_live_path_last_pose_stamp_ = 0U;
  manual_live_path_enabled_ = manual_live_path_points_.size() >= 2U;

  auto live_path_data = path_source_ ? path_source_->getPathData() : nullptr;
  if (!live_path_data)
  {
    manual_live_path_enabled_ = false;
    return;
  }

  if (!manual_live_path_enabled_)
  {
    live_path_data->reset();
    updateLivePathLayer();
    return;
  }

  ROBOGait::map::data::PathData::PathMetadata metadata;
  metadata.points = manual_live_path_points_;
  live_path_data->setPath(metadata);
  updateLivePathLayer();
}

void MapVisualizationManager::stopManualLivePath()
{
  manual_live_path_enabled_ = false;
  manual_live_path_points_.clear();
  manual_live_path_progress_index_ = 0U;
  manual_live_path_last_pose_stamp_ = 0U;

  const auto live_path_data = path_source_ ? path_source_->getPathData() : nullptr;
  if (live_path_data)
  {
    live_path_data->reset();
  }

  updateLivePathLayer();
}

void MapVisualizationManager::updateLivePathLayer()
{
  if (live_path_layer_)
  {
    live_path_layer_->update();
  }

  if (live_path_layer_item_)
  {
    live_path_layer_item_->update();
  }
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

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(laser_layer_item_);
  }

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

  if (map_layer_item_)
  {
    map_layer_item_->setSyncItem(particle_layer_item_);
  }

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
  emit viewTransformChanged();

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
  emit viewTransformChanged();

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
  const double width_m = static_cast<double>(metadata.width_) * metadata.resolution_;
  const double height_m = static_cast<double>(metadata.height_) * metadata.resolution_;

  if (width_m <= 0.0 || height_m <= 0.0)
  {
    qWarning() << "[MapVisualizationManager::fitToView] Invalid map dimensions";
    return;
  }

  const double origin_x = metadata.origin_x_;
  const double origin_y = metadata.origin_y_;
  const double theta = metadata.origin_theta_;
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
  emit viewTransformChanged();

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
  updateManualLivePath();
  updateAvailability();
}

void MapVisualizationManager::updateManualLivePath()
{
  if (!manual_live_path_enabled_ || manual_live_path_points_.size() < 2U)
  {
    return;
  }

  const auto robot_pose_data = pose_source_ ? pose_source_->getRobotPoseData() : nullptr;
  if (!robot_pose_data || !robot_pose_data->isAvailable())
  {
    return;
  }

  const uint64_t pose_stamp = robot_pose_data->getUpdateStamp();
  if (pose_stamp == manual_live_path_last_pose_stamp_)
  {
    return;
  }

  manual_live_path_last_pose_stamp_ = pose_stamp;
  const auto robot_pose = robot_pose_data->getMetadata();

  std::size_t nearest_index = manual_live_path_progress_index_;
  double nearest_distance_squared = std::numeric_limits<double>::max();

  for (std::size_t i = manual_live_path_progress_index_; i < manual_live_path_points_.size(); ++i)
  {
    const double dx = manual_live_path_points_[i].x_ - robot_pose.x_;
    const double dy = manual_live_path_points_[i].y_ - robot_pose.y_;
    const double distance_squared = dx * dx + dy * dy;

    if (distance_squared < nearest_distance_squared)
    {
      nearest_distance_squared = distance_squared;
      nearest_index = i;
    }
  }

  manual_live_path_progress_index_ = nearest_index;

  const double finish_distance_squared = MANUAL_LIVE_PATH_FINISH_DISTANCE_M * MANUAL_LIVE_PATH_FINISH_DISTANCE_M;
  if (nearest_index + 1U >= manual_live_path_points_.size() && nearest_distance_squared <= finish_distance_squared)
  {
    stopManualLivePath();
    return;
  }

  ROBOGait::map::data::PathData::PathMetadata metadata;
  metadata.points.reserve(manual_live_path_points_.size() - nearest_index + 1U);
  metadata.points.emplace_back(robot_pose.x_, robot_pose.y_);

  if (nearest_index + 1U < manual_live_path_points_.size())
  {
    metadata.points.insert(metadata.points.end(), manual_live_path_points_.begin() + static_cast<std::ptrdiff_t>(nearest_index + 1U),
                           manual_live_path_points_.end());
  }
  else
  {
    metadata.points.push_back(manual_live_path_points_.back());
  }

  const auto live_path_data = path_source_ ? path_source_->getPathData() : nullptr;
  if (live_path_data)
  {
    live_path_data->setPath(metadata);
  }

  updateLivePathLayer();
}

void MapVisualizationManager::createLayers()
{
  if (!is_initialized_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Not initialized";
    return;
  }

  if (!map_source_ || !pose_source_ || !path_source_ || !laser_source_ || !particle_source_)
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
  const auto live_path_data = path_source_->getPathData();
  const auto laser_data = laser_source_->getLaserScanData();
  const auto particle_data = particle_source_->getParticleCloudData();
  goal_robot_pose_data_ = std::make_shared<ROBOGait::map::data::RobotPoseData>();
  manual_path_data_ = std::make_shared<ROBOGait::map::data::PathData>();
  manual_draw_path_data_ = std::make_shared<ROBOGait::map::data::PathData>();

  if (spline_path_editor_)
  {
    spline_path_editor_->clearCachedPath();
  }

  map_layer_ = std::make_shared<ROBOGait::map::layer::MapLayer>();
  map_layer_->setMapData(map_data);

  robot_layer_ = std::make_shared<ROBOGait::map::layer::RobotLayer>();
  robot_layer_->setRobotPoseData(pose_data);
  robot_layer_->setRobotSize(robot_size_);

  goal_robot_layer_ = std::make_shared<ROBOGait::map::layer::RobotLayer>();
  goal_robot_layer_->setRobotPoseData(goal_robot_pose_data_);
  goal_robot_layer_->setRobotSize(robot_size_);

  path_layer_ = std::make_shared<ROBOGait::map::layer::PathLayer>();
  path_layer_->setPathData(manual_path_data_);

  manual_draw_path_layer_ = std::make_shared<ROBOGait::map::layer::PathLayer>();
  manual_draw_path_layer_->setPathData(manual_draw_path_data_);

  live_path_layer_ = std::make_shared<ROBOGait::map::layer::PathLayer>();
  live_path_layer_->setPathData(live_path_data);

  laser_layer_ = std::make_shared<ROBOGait::map::layer::LaserLayer>();
  laser_layer_->setLaserScanData(laser_data);

  particle_layer_ = std::make_shared<ROBOGait::map::layer::ParticleCloudLayer>();
  particle_layer_->setParticleCloudData(particle_data);

  if (!map_layer_ || !robot_layer_ || !goal_robot_layer_ || !path_layer_ || !manual_draw_path_layer_ || !live_path_layer_ || !laser_layer_ || !particle_layer_)
  {
    qCritical() << "[MapVisualizationManager::createLayers] Failed to create render layers";
    return;
  }

  render_scene_->setMapLayer(map_layer_);
  render_scene_->setRobotLayer(robot_layer_);
  render_scene_->setPathLayer(path_layer_);
  render_scene_->setLivePathLayer(live_path_layer_);
  render_scene_->setLaserLayer(laser_layer_);
  render_scene_->setParticleCloudLayer(particle_layer_);

  updateAvailability();

  qInfo() << "[MapVisualizationManager::createLayers] Layers created";
}

void MapVisualizationManager::destroyLayers()
{
  if (!map_layer_ && !robot_layer_ && !path_layer_ && !manual_draw_path_layer_ && !live_path_layer_ && !laser_layer_ && !particle_layer_)
  {
    if (spline_path_editor_)
    {
      spline_path_editor_->clearCachedPath();
    }

    qWarning() << "[MapVisualizationManager::destroyLayers] No layers to destroy";
    return;
  }

  if (!render_scene_)
  {
    if (spline_path_editor_)
    {
      spline_path_editor_->clearCachedPath();
    }

    qWarning() << "[MapVisualizationManager::destroyLayers] Render scene not available, cannot properly disconnect layers from scene";
    return;
  }

  render_scene_->stop();
  render_scene_->setMapLayer(nullptr);
  render_scene_->setRobotLayer(nullptr);
  render_scene_->setPathLayer(nullptr);
  render_scene_->setLivePathLayer(nullptr);
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
  if (goal_robot_layer_item_)
  {
    goal_robot_layer_item_->setRenderer(nullptr);
  }
  if (path_layer_item_)
  {
    path_layer_item_->setRenderer(nullptr);
  }
  if (manual_draw_path_layer_item_)
  {
    manual_draw_path_layer_item_->setRenderer(nullptr);
  }
  if (live_path_layer_item_)
  {
    live_path_layer_item_->setRenderer(nullptr);
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
  goal_robot_layer_.reset();
  path_layer_.reset();
  manual_draw_path_layer_.reset();
  live_path_layer_.reset();
  goal_robot_pose_data_.reset();
  manual_path_data_.reset();
  manual_draw_path_data_.reset();

  if (spline_path_editor_)
  {
    spline_path_editor_->clearCachedPath();
  }

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
      new_resolution = map_data->getMetadata().resolution_;
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
  emit viewTransformChanged();
  if (map_layer_item_)
  {
    map_layer_item_->update();
  }
  if (robot_layer_item_)
  {
    robot_layer_item_->update();
  }
}
