#include <iostream>

#include "Map/Source/RobotPoseSource.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::source;

RobotPoseSource::RobotPoseSource() : map_frame_(ROBOGait::ros::topics::TF_MAP_FRAME), robot_frame_(ROBOGait::ros::topics::TF_ROBOT_FRAME)
{
  parent_node_ = nullptr;
  initialized_ = false;
  active_ = false;
  context_ = std::nullopt;

  robot_pose_data_ = std::make_shared<ROBOGait::map::data::RobotPoseData>();
  tf_subscriber_ = std::make_shared<ROBOGait::map::subscribers::TFSubscriber>();
}

void RobotPoseSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    std::cerr << "[RobotPoseSource::initialize] Null parent node pointer" << std::endl;
    return;
  }

  parent_node_ = parent_node;
  tf_subscriber_->initialize(parent_node_, map_frame_, robot_frame_);
  tf_subscriber_->setRobotPoseData(robot_pose_data_.get());

  if (context_)
  {
    tf_subscriber_->setRobotContext(context_.value());
  }

  initialized_ = true;
}

void RobotPoseSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  if (tf_subscriber_)
  {
    tf_subscriber_->setRobotContext(context_.value());
  }
}

void RobotPoseSource::start()
{
  if (!initialized_ || !tf_subscriber_ || active_)
  {
    return;
  }

  tf_subscriber_->start();
  active_ = true;
}

void RobotPoseSource::stop()
{
  if (!active_ || !tf_subscriber_)
  {
    return;
  }

  tf_subscriber_->stop();
  robot_pose_data_->reset();
  active_ = false;
}

void RobotPoseSource::setPaused(bool paused)
{
  if (!tf_subscriber_)
  {
    std::cerr << "[RobotPoseSource::setPaused] TF subscriber is null" << std::endl;
    return;
  }

  tf_subscriber_->setPaused(paused);
}

bool RobotPoseSource::isActive() const { return active_; }

bool RobotPoseSource::isAvailable() const { return robot_pose_data_ && robot_pose_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::RobotPoseData> RobotPoseSource::getRobotPoseData() const { return robot_pose_data_; }
