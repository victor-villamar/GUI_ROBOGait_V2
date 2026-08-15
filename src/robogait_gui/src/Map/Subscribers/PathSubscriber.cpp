#include <vector>

#include <QDebug>

#include "Map/Subscribers/PathSubscriber.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::subscribers;

PathSubscriber::PathSubscriber() : parent_node_(nullptr), path_data_(nullptr), context_(std::nullopt), active_(false) {}

void PathSubscriber::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[PathSubscriber::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
}

void PathSubscriber::setPathData(data::PathData* path_data) { path_data_ = path_data; }

void PathSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context) { context_ = context; }

void PathSubscriber::start()
{
  if (!parent_node_)
  {
    qCritical() << "[PathSubscriber::start] Parent node is null";
    return;
  }

  if (active_)
  {
    return;
  }

  if (!context_)
  {
    qCritical() << "[PathSubscriber::start] Robot context is not set";
    return;
  }

  const std::string path_topic = context_->resolveTopic(ROBOGait::ros::topics::T_PLAN);

  sub_path_ = parent_node_->create_subscription<nav_msgs::msg::Path>(path_topic, ROBOGait::ros::QosProfiles::QOS_RELIABLE().keep_last(1),
                                                                     std::bind(&PathSubscriber::callbackPath, this, std::placeholders::_1));

  active_ = true;
}

void PathSubscriber::stop()
{
  if (!active_)
  {
    return;
  }

  sub_path_.reset();
  active_ = false;
}

bool PathSubscriber::isActive() const { return active_; }

void PathSubscriber::callbackPath(const nav_msgs::msg::Path::SharedPtr msg)
{
  if (!msg)
  {
    qWarning() << "[PathSubscriber::callbackPath] Received null message pointer";
    return;
  }

  if (!path_data_)
  {
    qCritical() << "[PathSubscriber::callbackPath] PathData is null";
    return;
  }

  if (msg->poses.empty())
  {
    path_data_->reset();
    return;
  }

  data::PathData::PathMetadata metadata;
  metadata.points.reserve(msg->poses.size());

  for (const auto& pose : msg->poses)
  {
    metadata.points.emplace_back(pose.pose.position.x, pose.pose.position.y);
  }

  path_data_->setPath(metadata);
}
