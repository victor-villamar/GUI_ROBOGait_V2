#include <QDebug>

#include "Map/Source/RobotPoseSource.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::source;

RobotPoseSource::RobotPoseSource() : map_frame_(TF_MAP_FRAME), robot_frame_(TF_ROBOT_FRAME)
{
  parent_node_ = nullptr;
  has_context_ = false;
  initialized_ = false;
  active_ = false;

  context_ = ROBOGait::context::RobotContext();
  robot_pose_data_ = std::make_shared<ROBOGait::map::data::RobotPoseData>();
  tf_subscriber_ = std::make_shared<ROBOGait::map::data::TFSubscriber>();
}

void RobotPoseSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[RobotPoseSource::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
  tf_subscriber_->initialize(parent_node_, map_frame_, robot_frame_);
  tf_subscriber_->setRobotPoseData(robot_pose_data_.get());

  if (has_context_)
  {
    tf_subscriber_->setRobotContext(context_);
  }

  initialized_ = true;
}

void RobotPoseSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;
  if (tf_subscriber_)
  {
    tf_subscriber_->setRobotContext(context_);
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

bool RobotPoseSource::isActive() const { return active_; }

bool RobotPoseSource::isAvailable() const { return robot_pose_data_ && robot_pose_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::RobotPoseData> RobotPoseSource::getRobotPoseData() const { return robot_pose_data_; }
