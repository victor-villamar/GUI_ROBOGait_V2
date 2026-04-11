#include <QDebug>

#include <rclcpp/create_timer.hpp>

#include "Robot/ManualControl.hpp"
#include "Ros/Define.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::robot::control;

ManualControl::ManualControl() :
    parent_node_(nullptr), pub_cmd_vel_(nullptr), timer_cmd_vel_(nullptr), topic_name_(""), linear_velocity_(0.0), angular_velocity_(0.0), timer_active_(false)
{
  qInfo() << "[ManualControl::ManualControl] Manual control created";
}

ManualControl::~ManualControl() { qInfo() << "[ManualControl::~ManualControl] Manual control destroyed"; }

void ManualControl::setROSNode(rclcpp::Node* node)
{
  if (node == nullptr)
  {
    qCritical() << "[ManualControl::setROSNode] Invalid ROS node provided";
    return;
  }

  parent_node_ = node;

  // Create timer for publishing velocity commands
  timer_cmd_vel_ = parent_node_->create_wall_timer(std::chrono::milliseconds(TIME_TO_PUBLISH_CMD_VEL),
                                                   std::bind(&ManualControl::publishVelocity, this)); // one-shot: false, autostart: false
  timer_cmd_vel_->cancel();
}

void ManualControl::setTopicName(const QString& topic_name)
{
  if (topic_name.isEmpty())
  {
    qCritical() << "[ManualControl::setTopicName] Empty topic name provided";
    return;
  }

  if (topic_name_ != topic_name && pub_cmd_vel_)
  {
    qInfo() << "[ManualControl::setTopicName] Topic changed, destroying old publisher";
    destroyPublisher();
  }

  topic_name_ = topic_name;
}

void ManualControl::destroyPublisher()
{
  if (timer_cmd_vel_)
  {
    timer_cmd_vel_->cancel();
    timer_active_ = false;
  }

  if (pub_cmd_vel_)
  {
    linear_velocity_ = 0.0;
    angular_velocity_ = 0.0;
    publishVelocity();
    qInfo() << "[ManualControl::destroyPublisher] Destroying publisher";
    pub_cmd_vel_.reset();
  }
}

void ManualControl::updateVelocity(double linear, double angular)
{
  // Clamp values to allowed ranges
  linear = std::max(-MAX_LINEAR_VELOCITY, std::min(MAX_LINEAR_VELOCITY, linear));
  angular = std::max(-MAX_ANGULAR_VELOCITY, std::min(MAX_ANGULAR_VELOCITY, angular));

  // Update internal values
  bool changed = false;
  if (linear_velocity_ != linear)
  {
    linear_velocity_ = linear;
    changed = true;
  }

  if (angular_velocity_ != angular)
  {
    angular_velocity_ = angular;
    changed = true;
  }

  if (timer_cmd_vel_ && !timer_active_)
  {
    timer_cmd_vel_->reset();
    timer_active_ = true;
  }

  if (changed)
  {
    emit velocityChanged();
  }
}

void ManualControl::stopRobot() { updateVelocity(0.0, 0.0); }

double ManualControl::getLinearVelocity() const { return linear_velocity_; }

double ManualControl::getAngularVelocity() const { return angular_velocity_; }

void ManualControl::ensurePublisherCreated()
{

  if (parent_node_ == nullptr)
  {
    qCritical() << "[ManualControl::ensurePublisherCreated] Cannot create publisher, parent_node_ is null";
    return;
  }

  if (topic_name_.isEmpty())
  {
    qCritical() << "[ManualControl::ensurePublisherCreated] Cannot create publisher, topic_name_ is empty";
    return;
  }

  if (pub_cmd_vel_)
  {
    return;
  }

  qInfo() << "[ManualControl::ensurePublisherCreated] Creating publisher for topic:" << topic_name_;
  pub_cmd_vel_ = parent_node_->create_publisher<geometry_msgs::msg::Twist>(topic_name_.toStdString(), ROBOGait::ros::QosProfiles::QOS_RELIABLE());
}

void ManualControl::publishVelocity()
{
  ensurePublisherCreated();

  if (!pub_cmd_vel_)
  {
    qWarning() << "[ManualControl::publishVelocity] Publisher could not be created";
    return;
  }

  // Create Twist message
  auto twist_msg = geometry_msgs::msg::Twist();
  twist_msg.linear.x = linear_velocity_;
  twist_msg.linear.y = 0.0;
  twist_msg.linear.z = 0.0;
  twist_msg.angular.x = 0.0;
  twist_msg.angular.y = 0.0;
  twist_msg.angular.z = angular_velocity_;

  // Publish message
  pub_cmd_vel_->publish(twist_msg);
}
