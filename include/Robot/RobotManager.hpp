#pragma once

#include <QObject>
#include <QString>
#include <rclcpp/node.hpp>

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
   * @brief Enables manual control mode for the selected robot
   */
  Q_INVOKABLE void enableManualControl();

  /**
   * @brief Disables manual control mode
   */
  Q_INVOKABLE void disableManualControl();

signals:
  void selectedRobotNamespaceChanged();
  void selectedRobotDisplayNameChanged();

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

  rclcpp::Node* parent_node_; /**< Pointer to the parent ROS node */

  std::unique_ptr<ROBOGait::robot::control::ManualControl> manual_control_; /**< Manual control instance */

  QString selected_robot_namespace_; /**< The namespace of the selected robot */
  QString cmd_vel_text_;             /**< The command velocity text */
  bool use_namespace_discovery_;     /**< True if selected robot is identified by namespace, false if by node name */
};
} // namespace manager
} // namespace robot
} // namespace ROBOGait
