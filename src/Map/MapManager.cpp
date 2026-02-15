#include <QDebug>
#include <cmath>

#include "Map/MapManager.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::manager;

MapManager& MapManager::getInstance()
{
  static MapManager instance;
  return instance;
}

MapManager::MapManager() :
    parent_node_(nullptr),
    sub_map_(nullptr),
    sub_map_updates_(nullptr),
    robot_pose_(nullptr),
    selected_robot_namespace_(""),
    use_namespace_discovery_(true),
    is_initialized_(false)
{
  qInfo() << "[MapManager::MapManager] Map manager created";
}

MapManager::~MapManager()
{
  destroySubscriptions();
  qInfo() << "[MapManager::~MapManager] Map manager destroyed";
}

void MapManager::setROSNode(rclcpp::Node* parent_node)
{
  if (is_initialized_)
  {
    qWarning() << "[MapManager::setROSNode] ROS node already set";
    return;
  }

  if (!parent_node)
  {
    qCritical() << "[MapManager::setROSNode] Null node pointer";
    return;
  }

  parent_node_ = parent_node;
  is_initialized_ = true;
}

bool MapManager::isInitialized() const { return is_initialized_; }

bool MapManager::isMapAvailable() const { return map_data_.isAvailable(); }

QImage MapManager::getMapImage() { return map_data_.toQImage(); }

int MapManager::getMapWidth() const
{
  if (!map_data_.isAvailable())
  {
    return 0;
  }
  return static_cast<int>(map_data_.getMetadata().width);
}

int MapManager::getMapHeight() const
{
  if (!map_data_.isAvailable())
  {
    return 0;
  }
  return static_cast<int>(map_data_.getMetadata().height);
}

double MapManager::getMapResolution() const
{
  if (!map_data_.isAvailable())
  {
    return 0.0;
  }
  return map_data_.getMetadata().resolution;
}

bool MapManager::isRobotPoseAvailable() const
{
  if (!robot_pose_)
  {
    qWarning() << "[MapManager::isRobotPoseAvailable] Robot pose is nullptr";
    return false;
  }

  return robot_pose_->isAvailable();
}

QPointF MapManager::getRobotScreenPos() const
{
  if (!robot_pose_ || !robot_pose_->isAvailable() || !map_data_.isAvailable())
  {
    return QPointF(0, 0);
  }

  int pixel_x = 0;
  int pixel_y = 0;

  if (map_data_.worldToPixel(robot_pose_->getX(), robot_pose_->getY(), pixel_x, pixel_y))
  {
    int screen_y = map_data_.getMetadata().height - pixel_y;
    return QPointF(static_cast<double>(pixel_x), static_cast<double>(screen_y));
  }

  return QPointF(0, 0);
}

double MapManager::getRobotScreenRotation() const
{
  if (!robot_pose_ || !robot_pose_->isAvailable())
  {
    return 0.0;
  }

  return -utils::rad2deg(robot_pose_->getTheta());
}

void MapManager::callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  if (!msg)
  {
    qWarning() << "[MapManager::callbackMap] Null message received";
    return;
  }

  const bool was_available = map_data_.isAvailable();

  // Update map data
  map_data_.updateFromOccupancyGrid(msg);

  if (!was_available && map_data_.isAvailable())
  {
    emit mapAvailableChanged();
    qInfo() << "[MapManager::callbackMap] Map now available";
  }

  emit mapMetadataChanged();
  emit mapImageChanged();

  // DEBUG
  // qDebug() << "[MapManager::callbackMap] Map updated:" << map_data_.getMetadata().width << "x" << map_data_.getMetadata().height
  //          << "cells, resolution:" << map_data_.getMetadata().resolution << "m/cell";
}

void MapManager::callbackMapUpdates(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg)
{
  if (!msg)
  {
    qWarning() << "[MapManager::callbackMapUpdates] Null message received";
    return;
  }

  map_data_.updateFromOccupancyGridUpdate(msg);

  emit mapImageChanged();

  // DEBUG
  // qDebug() << "[MapManager::callbackMapUpdates] Map updated incrementally:"
  //          << "region [" << msg->x << "," << msg->y << "] size [" << msg->width << "x" << msg->height << "]";
}

void MapManager::setSelectedRobot(const QString& robot_identifier, bool is_namespace)
{
  if (!is_initialized_)
  {
    qCritical() << "[MapManager::setSelectedRobot] Manager not initialized";
    return;
  }

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
      qCritical() << "[MapManager::setSelectedRobot] Invalid robot node name";
      return;
    }
  }

  if (normalized_identifier.isEmpty())
  {
    qCritical() << "[MapManager::setSelectedRobot] Invalid robot identifier";
    return;
  }

  // Store configuration
  selected_robot_namespace_ = normalized_identifier;
  use_namespace_discovery_ = is_namespace;

  // DEBUG
  // qDebug() << "[MapManager::setSelectedRobot] Robot configuration stored:" << normalized_identifier;
}

void MapManager::activateSubscriptions()
{
  if (!is_initialized_)
  {
    qCritical() << "[MapManager::activateSubscriptions] Manager not initialized";
    return;
  }

  if (!parent_node_)
  {
    qCritical() << "[MapManager::activateSubscriptions] Parent node is null";
    return;
  }

  if (selected_robot_namespace_.isEmpty())
  {
    qWarning() << "[MapManager::activateSubscriptions] No robot selected";
    return;
  }

  recreateSubscriptions();
}

void MapManager::destroySubscriptions()
{

  if (timer_robot_pose_)
  {
    timer_robot_pose_->cancel();
    timer_robot_pose_.reset();
    qInfo() << "[MapManager::destroySubscriptions] Robot pose timer stopped";
  }

  if (sub_map_)
  {
    sub_map_.reset();
    qInfo() << "[MapManager::destroySubscriptions] Map subscription destroyed";
  }

  if (sub_map_updates_)
  {
    sub_map_updates_.reset();
    qInfo() << "[MapManager::destroySubscriptions] Map updates subscription destroyed";
  }

  if (robot_pose_)
  {
    robot_pose_.reset();
    qInfo() << "[MapManager::destroySubscriptions] Robot pose destroyed";
  }

  if (map_data_.isAvailable())
  {
    map_data_ = MapData(); // Reset to empty map
    emit mapAvailableChanged();
    emit mapImageChanged();
    qInfo() << "[MapManager::destroySubscriptions] Map data cleared";
  }
}

QString MapManager::normalizeNamespace(const QString& robot_namespace) const
{
  QString normalized = robot_namespace.trimmed();

  if (normalized.isEmpty())
  {
    qWarning() << "[MapManager::normalizeNamespace] Empty namespace provided";
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

std::string MapManager::buildTopicName(const std::string& topic_suffix) const
{
  if (use_namespace_discovery_ && !selected_robot_namespace_.isEmpty())
  {
    return (selected_robot_namespace_.toStdString() + topic_suffix);
  }
  else
  {
    return topic_suffix;
  }
}

void MapManager::recreateSubscriptions()
{
  if (!parent_node_)
  {
    qCritical() << "[MapManager::recreateSubscriptions] Parent node is null";
    return;
  }

  destroySubscriptions();

  // Build topic names
  const std::string map_topic = buildTopicName(std::string(T_MAP));
  const std::string map_updates_topic = buildTopicName(std::string(T_MAP_UPDATES));

  // Subscriptions
  sub_map_ = parent_node_->create_subscription<nav_msgs::msg::OccupancyGrid>(map_topic, QOS_RELIABLE_LATCH,
                                                                             std::bind(&MapManager::callbackMap, this, std::placeholders::_1));

  sub_map_updates_ = parent_node_->create_subscription<map_msgs::msg::OccupancyGridUpdate>(
      map_updates_topic, QOS_RELIABLE, std::bind(&MapManager::callbackMapUpdates, this, std::placeholders::_1));

  std::string map_frame = TF_MAP_FRAME;
  std::string robot_frame = TF_ROBOT_FRAME;

  // Add namespace prefix to both map and robot frames for multi-robot systems
  if (use_namespace_discovery_ && !selected_robot_namespace_.isEmpty())
  {
    map_frame = selected_robot_namespace_.toStdString() + "/" + std::string(TF_MAP_FRAME);
    robot_frame = selected_robot_namespace_.toStdString() + "/" + std::string(TF_ROBOT_FRAME);
  }

  robot_pose_ = std::make_unique<RobotPose>(parent_node_, map_frame, robot_frame, 10.0);

  // Create timer to emit robot pose updates
  timer_robot_pose_ = parent_node_->create_wall_timer(std::chrono::milliseconds(TIME_TO_ROBOT_POSE_UPDATE),
                                                      std::bind(&MapManager::callbackRobotPoseSignalTimer, this)); // one-shot=false, auto-start=true
}

void MapManager::callbackRobotPoseSignalTimer()
{
  if (robot_pose_ && robot_pose_->isAvailable())
  {
    emit robotPoseChanged();
  }
}
