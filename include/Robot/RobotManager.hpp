#pragma once

#include <QObject>
#include <QString>
#include <rclcpp/callback_group.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/time.hpp>
#include <rclcpp/timer.hpp>
#include <std_msgs/msg/string.hpp>

#include "Robot/ManualControl.hpp"

namespace ROBOGait
{
namespace robot
{
namespace manager
{
class RobotManager : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of RobotManager class
   */
  RobotManager();

  /**
   * @brief Destructor of RobotManager class
   */
  ~RobotManager() override;

  // clang-format off
  Q_PROPERTY(QString selectedRobotNamespace
             READ getSelectedRobotNamespace
             NOTIFY selectedRobotNamespaceChanged
  )
  Q_PROPERTY(QString selectedRobotDisplayName
             READ getSelectedRobotDisplayName
             NOTIFY selectedRobotDisplayNameChanged
  )
  Q_PROPERTY(ROBOGait::robot::control::ManualControl* manualControl
             READ getManualControl
             CONSTANT
  )
  // clang-format on

  /**
   * @brief Get the namespace of the selected robot
   */
  QString getSelectedRobotNamespace() const;

  /**
   * @brief Get the display name of the selected robot
   */
  QString getSelectedRobotDisplayName() const;

  /**
   * @brief Get the manual control instance
   */
  ROBOGait::robot::control::ManualControl* getManualControl() const;

  /**
   * @brief Set the ROS node for the RobotManager
   *
   * @param parent_node Pointer to the parent ROS node
   */
  void setROSNode(rclcpp::Node* parent_node);

  /**
   * @brief Select a robot by its identifier (namespace or node name)
   *
   * @param robot_identifier The namespace or node name of the robot to select
   * @param is_namespace True if identifier is a namespace, false if it's a node name
   */
  void selectRobot(const QString& robot_identifier, bool is_namespace);

  /**
   * @brief Clear the selected robot
   */
  void clearSelection();

  /**
   * @brief Set whether to use namespace-based topic construction
   *
   * @param use_namespace_discovery True to use namespace, false to use node name
   */
  void setUseNamespaceDiscovery(bool use_namespace_discovery);

  /**
   * @brief Set whether to use topic filter for monitoring
   *
   * @param use_topic_filter True to monitor via topic, false to skip monitoring
   */
  void setUseTopicFilter(bool use_topic_filter);

  /**
   * @brief Enables manual control mode for the selected robot
   */
  Q_INVOKABLE void enableManualControl();

  /**
   * @brief Disables manual control mode
   */
  Q_INVOKABLE void disableManualControl();

  /**
   * @brief Check if selected robot is still available in discovery list
   * @param available_robots List of currently available robot namespaces
   */
  void checkRobotAvailability(const QStringList& available_robots);

signals:
  void selectedRobotNamespaceChanged();   /**< Emitted when the selected robot namespace changes */
  void selectedRobotDisplayNameChanged(); /**< Emitted when the selected robot display name changes */
  void robotDisconnected();               /**< Emitted when robot is disconnected due to timeout */

private:
  /**
   * @brief Normalize the robot namespace
   *
   * @param robot_namespace The namespace of the robot
   * @return The normalized namespace
   */
  QString normalizeNamespace(const QString& robot_namespace) const;

  /**
   * @brief Build topic name based on selected robot type
   *
   * @param topic_suffix The topic suffix (e.g., "/cmd_vel")
   *
   * @return The complete topic name
   */
  QString buildTopicName(const QString& topic_suffix) const;

  /**
   * @brief Callback for receiving robot_status messages
   *
   * @param msg Message received from robot
   */
  void callbackRobotStatus(const std_msgs::msg::String::SharedPtr msg);

  /**
   * @brief Check robot timeout and emit signal if disconnected
   */
  void callbackRobotTimeoutTimer();

  /**
   * @brief Start monitoring the selected robot
   */
  void startMonitoring();

  /**
   * @brief Stop monitoring the robot
   */
  void stopMonitoring();

  rclcpp::Node* parent_node_; /**< Pointer to the parent ROS node */

  std::unique_ptr<ROBOGait::robot::control::ManualControl> manual_control_; /**< Manual control instance */

  QString selected_robot_namespace_; /**< The namespace of the selected robot */
  bool use_namespace_discovery_;     /**< True if selected robot is identified by namespace, false if by node name */
  bool use_topic_filter_;            /**< True to monitor via topic, false to skip monitoring */

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_robot_status_; /**< Subscriber to robot_status topic */
  rclcpp::TimerBase::SharedPtr timer_robot_timeout_;                        /**< Timer for robot disconnection */
  rclcpp::CallbackGroup::SharedPtr cb_group_;                               /**< Callback group for subscriptions */
  rclcpp::Time last_robot_message_time_;                                    /**< Timestamp of last robot message */
  bool is_monitoring_;                                                      /**< Flag indicating if monitoring is active */

  static constexpr double TIMEOUT_SECONDS = 3.0; /**< Timeout in seconds for robot disconnection */
};
} // namespace manager
} // namespace robot
} // namespace ROBOGait
