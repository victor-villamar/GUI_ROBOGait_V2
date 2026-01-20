#pragma once

#include <QMetaType>
#include <QObject>
#include <QString>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>

Q_DECLARE_METATYPE(geometry_msgs::msg::Twist)

namespace ROBOGait
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
  Q_PROPERTY(QString cmdVelText
             READ getCmdVelText
             NOTIFY cmdVelTextChanged
  )
  Q_PROPERTY(QString selectedRobotDisplayName
             READ getSelectedRobotDisplayName
             NOTIFY selectedRobotDisplayNameChanged
  )
  // clang-format on

  /**
   * @brief Get the namespace of the selected robot
   */
  QString getSelectedRobotNamespace() const;

  /**
   * @brief Get the command velocity text
   */
  QString getCmdVelText() const;

  /**
   * @brief Get the display name of the selected robot
   */
  QString getSelectedRobotDisplayName() const;

  /**
   * @brief Set the ROS node for the RobotManager
   *
   * @param parent_node Pointer to the parent ROS node
   */
  void setROSNode(rclcpp::Node* parent_node);

  /**
   * @brief Select a robot by its namespace
   *
   * @param robot_namespace The namespace of the robot to select
   */
  Q_INVOKABLE void selectRobot(const QString& robot_namespace);

  /**
   * @brief Clear the selected robot
   */
  Q_INVOKABLE void clearSelection();

signals:
  void selectedRobotNamespaceChanged();
  void cmdVelTextChanged();
  void selectedRobotDisplayNameChanged();

  void cmdVelReceived(const geometry_msgs::msg::Twist& cmd_vel);

private slots:
  void onCmdVelReceived(const geometry_msgs::msg::Twist& cmd_vel);

private:
  /**
   * @brief Normalize the robot namespace
   *
   * @param robot_namespace The namespace of the robot
   * @return The normalized namespace
   */
  QString normalizeNamespace(const QString& robot_namespace) const;

  /**
   * @brief Subscribe to the command velocity topic
   */
  void subscribeToCmdVel();

  /**
   * @brief Callback for received command velocity messages
   *
   * @param cmd_vel The command velocity message
   */
  void callbackCmdVel(const geometry_msgs::msg::Twist& cmd_vel);

  /**
   * @brief Reset the command velocity state
   *
   * @param clear_text Whether to clear the command velocity text
   */
  void resetCmdVelState(bool clear_text);

  rclcpp::Node* parent_node_;                                              /**< Pointer to the parent ROS node */
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_cmd_vel_; /**< Subscriber to the command velocity topic */

  QString selected_robot_namespace_; /**< The namespace of the selected robot */
  QString cmd_vel_text_;             /**< The command velocity text */
  double last_linear_x_;             /**< The last linear x velocity */
  double last_linear_y_;             /**< The last linear y velocity */
  double last_linear_z_;             /**< The last linear z velocity */
  double last_angular_x_;            /**< The last angular x velocity */
  double last_angular_y_;            /**< The last angular y velocity */
  double last_angular_z_;            /**< The last angular z velocity */
  bool has_cmd_vel_;
};
} // namespace manager
} // namespace ROBOGait
