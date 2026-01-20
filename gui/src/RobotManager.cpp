#include <QDebug>
#include <cmath>

#include "Define.hpp"
#include "RobotManager.hpp"
#include "TopicsName.hpp"

using namespace ROBOGait::manager;

RobotManager::RobotManager() : parent_node_(nullptr), selected_robot_namespace_(""), cmd_vel_text_(""), sub_cmd_vel_(nullptr)
{
  qInfo() << "[RobotManager::RobotManager] RobotManager created";
  // clang-format off
  connect(this,
          &RobotManager::cmdVelReceived,
          this,
          &RobotManager::onCmdVelReceived,
          Qt::QueuedConnection
  );
  // clang-format on
  last_linear_x_ = 0.0;
  last_linear_y_ = 0.0;
  last_linear_z_ = 0.0;
  last_angular_x_ = 0.0;
  last_angular_y_ = 0.0;
  last_angular_z_ = 0.0;
  has_cmd_vel_ = false;
}

RobotManager::~RobotManager()
{
  qInfo() << "[RobotManager::~RobotManager] RobotManager destroyed";
  clearSelection();
}

QString RobotManager::getSelectedRobotNamespace() const { return selected_robot_namespace_; }

QString RobotManager::getCmdVelText() const { return cmd_vel_text_; }

QString RobotManager::getSelectedRobotDisplayName() const
{
  QString ns = selected_robot_namespace_;
  if (ns.startsWith('/'))
  {
    ns.remove(0, 1);
  }
  return ns.replace("_", " ");
}

void RobotManager::setROSNode(rclcpp::Node* parent_node)
{
  if (parent_node == nullptr)
  {
    qCritical() << "[RobotManager::setROSNode] No valid ROS node provided.";
    return;
  }
  parent_node_ = parent_node;
}

void RobotManager::selectRobot(const QString& robot_namespace)
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotManager::selectRobot] ROS node is not set. Cannot select robot.";
    return;
  }

  const QString normalized_namespace = normalizeNamespace(robot_namespace);

  if (normalized_namespace.isEmpty())
  {
    qCritical() << "[RobotManager::selectRobot] Invalid robot namespace.";
    clearSelection();
    return;
  }

  if (selected_robot_namespace_ != normalized_namespace)
  {
    selected_robot_namespace_ = normalized_namespace;
    emit selectedRobotNamespaceChanged();
    emit selectedRobotDisplayNameChanged();
  }

  subscribeToCmdVel();
}

void RobotManager::clearSelection()
{
  const QString topic_name = selected_robot_namespace_ + QString::fromUtf8(T_TB3_CMD_VEL);

  if (sub_cmd_vel_)
  {
    qInfo() << "[RobotManager::clearSelection] Unsubscribing from topic:" << topic_name;
    sub_cmd_vel_.reset();
  }

  resetCmdVelState(true);

  if (!selected_robot_namespace_.isEmpty())
  {
    selected_robot_namespace_.clear();
    emit selectedRobotNamespaceChanged();
    emit selectedRobotDisplayNameChanged();
  }
}

void RobotManager::onCmdVelReceived(const geometry_msgs::msg::Twist& cmd_vel)
{
  constexpr double eps = 1e-3;

  bool changed = !has_cmd_vel_;

  if (!changed)
  {
    changed = (std::abs(cmd_vel.linear.x - last_linear_x_) > eps) || (std::abs(cmd_vel.linear.y - last_linear_y_) > eps) ||
              (std::abs(cmd_vel.linear.z - last_linear_z_) > eps) || (std::abs(cmd_vel.angular.x - last_angular_x_) > eps) ||
              (std::abs(cmd_vel.angular.y - last_angular_y_) > eps) || (std::abs(cmd_vel.angular.z - last_angular_z_) > eps);
  }

  if (!changed)
  {
    return;
  }

  has_cmd_vel_ = true;

  last_linear_x_ = cmd_vel.linear.x;
  last_linear_y_ = cmd_vel.linear.y;
  last_linear_z_ = cmd_vel.linear.z;
  last_angular_x_ = cmd_vel.angular.x;
  last_angular_y_ = cmd_vel.angular.y;
  last_angular_z_ = cmd_vel.angular.z;

  const QString text = QString("Linear: [x: %1, y: %2, z: %3]  |  Angular: [x: %4, y: %5, z: %6]")
                           .arg(cmd_vel.linear.x, 0, 'f', 2)
                           .arg(cmd_vel.linear.y, 0, 'f', 2)
                           .arg(cmd_vel.linear.z, 0, 'f', 2)
                           .arg(cmd_vel.angular.x, 0, 'f', 2)
                           .arg(cmd_vel.angular.y, 0, 'f', 2)
                           .arg(cmd_vel.angular.z, 0, 'f', 2);

  if (cmd_vel_text_ != text)
  {
    cmd_vel_text_ = text;
    emit cmdVelTextChanged();
  }
}

QString RobotManager::normalizeNamespace(const QString& robot_namespace) const
{
  QString normalized = robot_namespace.trimmed();
  if (normalized.isEmpty())
  {
    qWarning() << "[RobotManager::normalizeNamespace] Empty namespace provided.";
    return QString();
  }
  if (!normalized.startsWith('/'))
  {
    normalized.prepend('/');
  }

  while (normalized.size() > 1 && normalized.endsWith('/'))
  {
    normalized.chop(1);
  }

  return normalized;
}

void RobotManager::subscribeToCmdVel()
{
  if (sub_cmd_vel_)
  {
    sub_cmd_vel_.reset();
  }

  resetCmdVelState(true);

  const QString topic_name = selected_robot_namespace_ + QString::fromUtf8(T_TB3_CMD_VEL);

  // Create subscription
  sub_cmd_vel_ = parent_node_->create_subscription<geometry_msgs::msg::Twist>(topic_name.toStdString(), QOS_RELIABLE,
                                                                              std::bind(&RobotManager::callbackCmdVel, this, std::placeholders::_1));

  qInfo() << "[RobotManager::subscribeToCmdVel] Subscribed to topic:" << topic_name;
}

void RobotManager::callbackCmdVel(const geometry_msgs::msg::Twist& cmd_vel) { emit cmdVelReceived(cmd_vel); }

void RobotManager::resetCmdVelState(bool clear_text)
{
  has_cmd_vel_ = false;

  last_linear_x_ = 0.0;
  last_linear_y_ = 0.0;
  last_linear_z_ = 0.0;
  last_angular_x_ = 0.0;
  last_angular_y_ = 0.0;
  last_angular_z_ = 0.0;

  if (clear_text && !cmd_vel_text_.isEmpty())
  {
    cmd_vel_text_.clear();
    emit cmdVelTextChanged();
  }
}