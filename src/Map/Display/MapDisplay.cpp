#include <QDebug>
#include <QPainter>

#include "Map/Display/MapDisplay.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::display;

MapDisplay::MapDisplay(QObject* parent) :
    BaseDisplay("MapDisplay", parent), map_data_(nullptr), map_graphics_dirty_(false), selected_robot_namespace_(""), use_namespace_discovery_(true)
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
  map_data_ = std::make_shared<MapData>();

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
  QString normalized_identifier;

  if (is_namespace)
  {
    normalized_identifier = normalizeNamespace(robot_identifier);
  }
  else
  {
    normalized_identifier = robot_identifier.trimmed();

    if (normalized_identifier.isEmpty())
    {
      qCritical() << "[MapDisplay::setSelectedRobot] Invalid robot node name";
      return;
    }
  }

  if (normalized_identifier.isEmpty())
  {
    qCritical() << "[MapDisplay::setSelectedRobot] Invalid robot identifier";
    return;
  }

  // Store configuration
  selected_robot_namespace_ = normalized_identifier;
  use_namespace_discovery_ = is_namespace;
}

void MapDisplay::activateSubscriptions()
{
  if (!parent_node_)
  {
    qCritical() << "[MapDisplay::activateSubscriptions] Parent node is null";
    return;
  }

  if (selected_robot_namespace_.isEmpty())
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
    qWarning() << "[MapDisplay::callbackMap] MapData is null";
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

  // Use MapData's built-in QImage conversion
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

QString MapDisplay::normalizeNamespace(const QString& robot_namespace) const
{
  QString normalized = robot_namespace.trimmed();

  if (normalized.isEmpty())
  {
    qWarning() << "[MapDisplay::normalizeNamespace] Empty namespace provided";
    return QString();
  }

  if (!normalized.startsWith('/'))
  {
    normalized.prepend('/');
  }

  while (normalized.size() > 1 && normalized.endsWith('/'))
  {
    normalized.chop(1);
  }

  normalized.replace(" ", "_");

  return normalized;
}

std::string MapDisplay::buildTopicName(const std::string& topic_suffix) const
{
  if (use_namespace_discovery_ && !selected_robot_namespace_.isEmpty())
  {
    // Multi-robot: namespace + topic
    return (selected_robot_namespace_.toStdString() + topic_suffix);
  }
  else
  {
    // Single robot: topic only
    return topic_suffix;
  }
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

  // Build topic names based on namespace configuration
  const std::string map_topic = buildTopicName(std::string(T_MAP));
  const std::string map_updates_topic = buildTopicName(std::string(T_MAP_UPDATES));

  // Create subscriptions
  sub_map_ = parent_node_->create_subscription<nav_msgs::msg::OccupancyGrid>(map_topic, QOS_RELIABLE_LATCH.keep_last(1),
                                                                             std::bind(&MapDisplay::callbackMap, this, std::placeholders::_1));

  sub_map_update_ = parent_node_->create_subscription<map_msgs::msg::OccupancyGridUpdate>(
      map_updates_topic, QOS_RELIABLE.keep_last(10), std::bind(&MapDisplay::callbackMapUpdate, this, std::placeholders::_1));

  qInfo() << "[MapDisplay::recreateSubscriptions] Subscriptions created successfully";
}
