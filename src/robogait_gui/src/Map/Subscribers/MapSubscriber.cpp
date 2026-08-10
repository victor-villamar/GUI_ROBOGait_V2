#include <cstdint>
#include <iostream>
#include <vector>

#include "Map/Subscribers/MapSubscriber.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::subscribers;

MapSubscriber::MapSubscriber() : parent_node_(nullptr), map_data_(nullptr), context_(std::nullopt), active_(false) {}

void MapSubscriber::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    std::cerr << "[MapSubscriber::initialize] Null parent node pointer" << std::endl;
    return;
  }

  parent_node_ = parent_node;
}

void MapSubscriber::setMapData(data::MapData* map_data) { map_data_ = map_data; }

void MapSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context) { context_ = context; }

void MapSubscriber::start()
{
  if (!parent_node_)
  {
    std::cerr << "[MapSubscriber::start] Parent node is null" << std::endl;
    return;
  }

  if (active_)
  {
    return;
  }

  if (!context_)
  {
    std::cerr << "[MapSubscriber::start] Robot context is not set" << std::endl;
    return;
  }

  const std::string map_topic = context_->resolveTopic(ROBOGait::ros::topics::T_MAP);
  const std::string map_updates_topic = context_->resolveTopic(ROBOGait::ros::topics::T_MAP_UPDATES);

  sub_map_ = parent_node_->create_subscription<nav_msgs::msg::OccupancyGrid>(map_topic, ROBOGait::ros::QosProfiles::QOS_RELIABLE_LATCH().keep_last(1),
                                                                             std::bind(&MapSubscriber::callbackMap, this, std::placeholders::_1));

  sub_map_update_ = parent_node_->create_subscription<map_msgs::msg::OccupancyGridUpdate>(
      map_updates_topic, ROBOGait::ros::QosProfiles::QOS_RELIABLE().keep_last(10), std::bind(&MapSubscriber::callbackMapUpdate, this, std::placeholders::_1));

  active_ = true;
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
}

bool MapSubscriber::isActive() const { return active_; }

void MapSubscriber::callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{

  if (!msg)
  {
    std::cerr << "[MapSubscriber::callbackMap] Received null message pointer" << std::endl;
    return;
  }

  data::MapData::MapMetadata metadata;
  metadata.resolution_ = static_cast<double>(msg->info.resolution);
  metadata.width_ = msg->info.width;
  metadata.height_ = msg->info.height;
  metadata.origin_x_ = msg->info.origin.position.x;
  metadata.origin_y_ = msg->info.origin.position.y;
  metadata.origin_theta_ = utils::getYaw(msg->info.origin.orientation);

  std::vector<int8_t> occupancy_data(msg->data.begin(), msg->data.end());

  if (!map_data_)
  {
    std::cerr << "[MapSubscriber::callbackMap] MapData is null" << std::endl;
    return;
  }

  map_data_->setOccupancyData(occupancy_data, metadata);
}

void MapSubscriber::callbackMapUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg)
{
  if (!msg)
  {
    std::cerr << "[MapSubscriber::callbackMapUpdate] Received null message pointer" << std::endl;
    return;
  }

  if (!map_data_)
  {
    std::cerr << "[MapSubscriber::callbackMapUpdate] MapData is null" << std::endl;
    return;
  }

  if (!map_data_->isAvailable())
  {
    std::cerr << "[MapSubscriber::callbackMapUpdate] MapData is not available" << std::endl;
    return;
  }

  std::vector<int8_t> update_data(msg->data.begin(), msg->data.end());
  map_data_->updateRegion(msg->x, msg->y, msg->width, msg->height, update_data);
}
