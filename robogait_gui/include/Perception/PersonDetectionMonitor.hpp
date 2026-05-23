#pragma once

#include <QObject>

#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>

#include <navigation_pkg/msg/user.hpp>

namespace ROBOGait
{
namespace perception
{
namespace monitor
{
/**
 * @brief Monitors person detections from ROS topic and notifies QML.
 */
class PersonDetectionMonitor : public QObject
{
  Q_OBJECT

  // clang-format off
  Q_PROPERTY(bool monitoring
             READ isMonitoring
             NOTIFY monitoringChanged)
  // clang-format on

public:
  explicit PersonDetectionMonitor(QObject* parent = nullptr);

  void setROSNode(rclcpp::Node* parent_node);
  void setSelectedRobot(const QString& robot_namespace, bool use_namespace_discovery);

  Q_INVOKABLE bool startMonitoring();
  Q_INVOKABLE void stopMonitoring();
  Q_INVOKABLE bool isMonitoring() const;

signals:
  void monitoringChanged();            // Emitted when monitoring state changes
  void personDetected(int detections); // Emitted when a new person detection message is received, with the number of detections

private:
  void callbackUserDetection(const navigation_pkg::msg::User::SharedPtr msg);

  rclcpp::Node* parent_node_;
  QString selected_robot_namespace_;
  bool use_namespace_discovery_;
  rclcpp::Subscription<navigation_pkg::msg::User>::SharedPtr sub_user_detection_;
  bool is_monitoring_;

  static constexpr int NOT_DETECTED = 0; /**< Constant representing no detections */
};
} // namespace monitor
} // namespace perception
} // namespace ROBOGait
