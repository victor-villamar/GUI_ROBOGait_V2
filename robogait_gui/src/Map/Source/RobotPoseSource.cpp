#include <QDebug>

#include "Map/Source/RobotPoseSource.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::source;

RobotPoseSource::RobotPoseSource() : robot_pose_data_(nullptr), map_frame_(TF_MAP_FRAME), robot_frame_(TF_ROBOT_FRAME)
{
  parent_node_ = nullptr;
  context_ = ROBOGait::context::RobotContext();
  has_context_ = false;
  initialized_ = false;
  active_ = false;
}

void RobotPoseSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[RobotPoseSource::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;

  if (!robot_pose_data_)
  {
    robot_pose_data_ = std::make_shared<ROBOGait::map::data::RobotPoseData>(parent_node_, map_frame_, robot_frame_);
  }

  if (has_context_ && robot_pose_data_)
  {
    robot_pose_data_->setRobotContext(context_);
  }

  initialized_ = true;
}

void RobotPoseSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;
  if (robot_pose_data_)
  {
    robot_pose_data_->setRobotContext(context_);
  }
}

void RobotPoseSource::start()
{
  if (!initialized_ || !robot_pose_data_ || active_)
  {
    return;
  }

  robot_pose_data_->setEnabled(true);
  active_ = true;
}

void RobotPoseSource::stop()
{
  if (!active_ || !robot_pose_data_)
  {
    return;
  }

  robot_pose_data_->setEnabled(false);
  robot_pose_data_->reset();
  active_ = false;
}

bool RobotPoseSource::isActive() const { return active_; }

bool RobotPoseSource::isAvailable() const { return robot_pose_data_ && robot_pose_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::RobotPoseData> RobotPoseSource::getRobotPoseData() const { return robot_pose_data_; }
