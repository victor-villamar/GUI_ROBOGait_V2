#include <functional>

#include <QDebug>

#include "Context/RobotContext.hpp"
#include "Perception/PersonDetectionMonitor.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::perception::monitor;

PersonDetectionMonitor::PersonDetectionMonitor(QObject* parent) :
    QObject(parent), parent_node_(nullptr), selected_robot_namespace_(), use_namespace_discovery_(true), sub_user_detection_(nullptr), is_monitoring_(false)
{
}

void PersonDetectionMonitor::setROSNode(rclcpp::Node* parent_node)
{
  if (parent_node_ == parent_node)
  {
    return;
  }

  stopMonitoring();
  parent_node_ = parent_node;
}

void PersonDetectionMonitor::setSelectedRobot(const QString& robot_namespace, bool use_namespace_discovery)
{
  if (is_monitoring_)
  {
    stopMonitoring();
  }

  selected_robot_namespace_ = robot_namespace;
  use_namespace_discovery_ = use_namespace_discovery;
}

bool PersonDetectionMonitor::startMonitoring()
{
  if (is_monitoring_)
  {
    return true;
  }

  if (!parent_node_)
  {
    qCritical() << "[PersonDetectionMonitor::startMonitoring] Parent ROS node is not set";
    return false;
  }

  ROBOGait::context::RobotContext context;

  if (!context.setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_))
  {
    qCritical() << "[PersonDetectionMonitor::startMonitoring] Robot context is not configured";
    return false;
  }

  const std::string full_topic = context.resolveTopic(T_USER_DETECTION);

  sub_user_detection_ = parent_node_->create_subscription<navigation_pkg::msg::User>(
      full_topic, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT(), std::bind(&PersonDetectionMonitor::callbackUserDetection, this, std::placeholders::_1));

  is_monitoring_ = true;
  emit monitoringChanged();

  qInfo() << "[PersonDetectionMonitor::startMonitoring] Monitoring started on topic:" << full_topic.c_str();
  return true;
}

void PersonDetectionMonitor::stopMonitoring()
{
  if (!is_monitoring_ && !sub_user_detection_)
  {
    return;
  }

  sub_user_detection_.reset();

  if (is_monitoring_)
  {
    is_monitoring_ = false;
    emit monitoringChanged();
  }

  qInfo() << "[PersonDetectionMonitor::stopMonitoring] Monitoring stopped";
}

bool PersonDetectionMonitor::isMonitoring() const { return is_monitoring_; }

void PersonDetectionMonitor::callbackUserDetection(const navigation_pkg::msg::User::SharedPtr msg)
{
  if (!is_monitoring_)
  {
    return;
  }

  if (!msg)
  {
    return;
  }

  if (msg->detection <= NOT_DETECTED)
  {
    return;
  }

  const int detections = msg->detection;

  emit personDetected(detections);
  stopMonitoring();
}
