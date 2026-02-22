#include <QDebug>
#include <QPainter>

#include "Map/Display/MapDisplay.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::display;

MapDisplay::MapDisplay(QObject* parent) :
    BaseDisplay("MapDisplay", parent), map_data_(nullptr), map_graphics_dirty_(false), has_context_(false)
{
  // Create graphics item
  map_item_ = std::make_unique<QGraphicsPixmapItem>();
  map_item_->setTransformationMode(Qt::SmoothTransformation);

  qInfo() << "[MapDisplay::MapDisplay] Map display created";
}

MapDisplay::~MapDisplay() { shutdown(); }

void MapDisplay::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[MapDisplay::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;

  // Create map data container
  map_data_ = std::make_shared<ROBOGait::map::data::MapLayerData>();
  if (has_context_)
  {
    map_data_->setRobotContext(context_);
  }

  qInfo() << "[MapDisplay::initialize] Map display initialized";
}

void MapDisplay::shutdown()
{
  if (sub_map_)
  {
    sub_map_.reset();
  }

  if (sub_map_update_)
  {
    sub_map_update_.reset();
  }

  qInfo() << "[MapDisplay::shutdown] Map display shutdown";
}

void MapDisplay::update(double wall_dt, double ros_dt)
{
  (void)wall_dt; // Unused
  (void)ros_dt;  // Unused

  // Update graphics if map data changed
  if (map_graphics_dirty_)
  {
    QMutexLocker lock(&data_mutex_);
    updateMapGraphics();
    map_graphics_dirty_ = false;
  }
}

void MapDisplay::setSelectedRobot(const QString& robot_identifier, bool is_namespace)
{
  ROBOGait::context::RobotContext context;
  if (!context.setSelectedRobot(robot_identifier, is_namespace))
  {
    qCritical() << "[MapDisplay::setSelectedRobot] Invalid robot identifier";
    return;
  }

  setRobotContext(context);
}

void MapDisplay::activateSubscriptions()
{
  if (!parent_node_)
  {
    qCritical() << "[MapDisplay::activateSubscriptions] Parent node is null";
    return;
  }

  if (!has_context_)
  {
    qWarning() << "[MapDisplay::activateSubscriptions] No robot selected, using default topics";
  }

  recreateSubscriptions();
}

void MapDisplay::callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  QMutexLocker lock(&data_mutex_);

  if (!map_data_)
  {
    qWarning() << "[MapDisplay::callbackMap] MapLayerData is null";
    return;
  }

  // Update map data
  map_data_->updateFromOccupancyGrid(msg);

  // DEBUG
  // qDebug() << "[MapDisplay::callbackMap] Map received:" << msg->info.width << "x" << msg->info.height << "Resolution:" << msg->info.resolution << "m/px";

  // Mark graphics as dirty
  map_graphics_dirty_ = true;

  emit mapUpdated();
  emit mapMetadataChanged();
  emit renderRequested();
}

void MapDisplay::callbackMapUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg)
{
  QMutexLocker lock(&data_mutex_);

  if (!map_data_ || !map_data_->isAvailable())
  {
    qWarning() << "[MapDisplay::callbackMapUpdate] Map not initialized, ignoring update";
    return;
  }

  // Apply incremental update
  map_data_->updateFromOccupancyGridUpdate(msg);

  // Mark graphics as dirty
  map_graphics_dirty_ = true;

  emit mapUpdated();
  emit renderRequested();
}

void MapDisplay::updateMapGraphics()
{
  if (!map_data_ || !map_data_->isAvailable())
  {
    return;
  }

  // Use MapLayerData's built-in QImage conversion
  QImage map_image = map_data_->toQImage();

  if (map_image.isNull())
  {
    qWarning() << "[MapDisplay::updateMapGraphics] Failed to create map image";
    return;
  }

  // Update pixmap
  map_item_->setPixmap(QPixmap::fromImage(map_image));

  // Position the item at map origin
  auto metadata = map_data_->getMetadata();
  double origin_x = metadata.origin_x;
  double origin_y = metadata.origin_y;

  double scene_x = origin_x;
  double scene_y = -(origin_y + metadata.height * metadata.resolution); // Flip Y axis, more accurate to ROS conventions

  map_item_->setPos(scene_x, scene_y);
  map_item_->setScale(metadata.resolution);

  // DEBUG
  // qDebug() << "[MapDisplay::updateMapGraphics] Map graphics updated. Position:" << scene_x << "," << scene_y << "Scale:" << metadata.resolution;
}

void MapDisplay::recreateSubscriptions()
{
  if (!parent_node_)
  {
    qCritical() << "[MapDisplay::recreateSubscriptions] Parent node is null";
    return;
  }

  // Destroy existing subscriptions
  if (sub_map_)
  {
    sub_map_.reset();
  }

  if (sub_map_update_)
  {
    sub_map_update_.reset();
  }

  const std::string map_topic = map_data_ ? map_data_->mapTopic() : std::string(T_MAP);
  const std::string map_updates_topic = map_data_ ? map_data_->mapUpdatesTopic() : std::string(T_MAP_UPDATES);

  // Create subscriptions
  sub_map_ = parent_node_->create_subscription<nav_msgs::msg::OccupancyGrid>(map_topic, QOS_RELIABLE_LATCH.keep_last(1),
                                                                             std::bind(&MapDisplay::callbackMap, this, std::placeholders::_1));

  sub_map_update_ = parent_node_->create_subscription<map_msgs::msg::OccupancyGridUpdate>(
      map_updates_topic, QOS_RELIABLE.keep_last(10), std::bind(&MapDisplay::callbackMapUpdate, this, std::placeholders::_1));

  qInfo() << "[MapDisplay::recreateSubscriptions] Subscriptions created successfully";
}

void MapDisplay::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;

  if (map_data_)
  {
    map_data_->setRobotContext(context_);
  }
}
