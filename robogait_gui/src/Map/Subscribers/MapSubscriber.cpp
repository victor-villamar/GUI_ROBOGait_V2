#include <QDebug>

#include "Map/Subscribers/MapSubscriber.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::data;

MapSubscriber::MapSubscriber() : parent_node_(nullptr), has_context_(false), active_(false) {}

void MapSubscriber::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[MapSubscriber::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
}

void MapSubscriber::setMapData(const std::shared_ptr<MapData>& map_data)
{
  map_data_ = map_data;
  if (map_data_ && has_context_)
  {
    map_data_->setRobotContext(context_);
  }
}

void MapSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;
  if (map_data_)
  {
    map_data_->setRobotContext(context_);
  }
}

void MapSubscriber::start()
{
  if (!parent_node_)
  {
    qCritical() << "[MapSubscriber::start] Parent node is null";
    return;
  }

  if (active_)
  {
    return;
  }

  const std::string map_topic = map_data_ ? map_data_->mapTopic() : std::string(T_MAP);
  const std::string map_updates_topic = map_data_ ? map_data_->mapUpdatesTopic() : std::string(T_MAP_UPDATES);

  sub_map_ = parent_node_->create_subscription<nav_msgs::msg::OccupancyGrid>(map_topic, QOS_RELIABLE_LATCH.keep_last(1),
                                                                             std::bind(&MapSubscriber::callbackMap, this, std::placeholders::_1));

  sub_map_update_ = parent_node_->create_subscription<map_msgs::msg::OccupancyGridUpdate>(
      map_updates_topic, QOS_RELIABLE.keep_last(10), std::bind(&MapSubscriber::callbackMapUpdate, this, std::placeholders::_1));

  active_ = true;
  qInfo() << "[MapSubscriber::start] Subscribed to map topics";
}

void MapSubscriber::stop()
{
  if (!active_)
  {
    return;
  }

  sub_map_.reset();
  sub_map_update_.reset();
  active_ = false;

  qInfo() << "[MapSubscriber::stop] Subscriptions stopped";
}

bool MapSubscriber::isActive() const { return active_; }

void MapSubscriber::callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  if (!map_data_)
  {
    qWarning() << "[MapSubscriber::callbackMap] MapData is null";
    return;
  }

  map_data_->updateFromOccupancyGrid(msg);
}

void MapSubscriber::callbackMapUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg)
{
  if (!map_data_)
  {
    qWarning() << "[MapSubscriber::callbackMapUpdate] MapData is null";
    return;
  }

  map_data_->updateFromOccupancyGridUpdate(msg);
}
