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
  RobotManager();
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
  // clang-format on

  QString getSelectedRobotNamespace() const;
  QString getCmdVelText() const;

  void setROSNode(rclcpp::Node* parent_node);

  Q_INVOKABLE void selectRobot(const QString& robot_namespace);
  Q_INVOKABLE void clearSelection();

signals:
  void selectedRobotNamespaceChanged();
  void cmdVelTextChanged();

  void cmdVelReceived(const geometry_msgs::msg::Twist& cmd_vel);

private slots:
  void onCmdVelReceived(const geometry_msgs::msg::Twist& cmd_vel);

private:
  QString normalizeNamespace(const QString& robot_namespace) const;
  void subscribeToCmdVel();
  void callbackCmdVel(const geometry_msgs::msg::Twist& cmd_vel);
  void resetCmdVelState(bool clear_text);

  rclcpp::Node* parent_node_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_cmd_vel_;

  QString selected_robot_namespace_;
  QString cmd_vel_text_;
  double last_linear_x_;
  double last_linear_y_;
  double last_linear_z_;
  double last_angular_x_;
  double last_angular_y_;
  double last_angular_z_;
  bool has_cmd_vel_;
};
} // namespace manager
} // namespace ROBOGait
