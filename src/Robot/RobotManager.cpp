#include <QDebug>
#include <cmath>

#include "Define.hpp"
#include "Robot/RobotManager.hpp"
#include "TopicsName.hpp"

using namespace ROBOGait::manager;

RobotManager::RobotManager() : parent_node_(nullptr), selected_robot_namespace_(""), use_namespace_discovery_(true)
{
  qInfo() << "[RobotManager::RobotManager] RobotManager created";

  manual_control_ = std::make_unique<ROBOGait::control::ManualControl>();
}

RobotManager::~RobotManager()
{
  qInfo() << "[RobotManager::~RobotManager] RobotManager destroyed";
  clearSelection();
}

QString RobotManager::getSelectedRobotNamespace() const { return selected_robot_namespace_; }

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

  manual_control_->setROSNode(parent_node);
}

void RobotManager::selectRobot(const QString& robot_identifier, bool is_namespace)
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotManager::selectRobot] ROS node is not set. Cannot select robot.";
    return;
  }

  QString normalized_identifier;

  if (is_namespace)
  {
    normalized_identifier = normalizeNamespace(robot_identifier);
  }
  else
  {
    normalized_identifier = robot_identifier.trimmed();
    if (normalized_identifier.isEmpty())
    {
      qCritical() << "[RobotManager::selectRobot] Invalid robot node name.";
      clearSelection();
      return;
    }
  }

  if (normalized_identifier.isEmpty())
  {
    qCritical() << "[RobotManager::selectRobot] Invalid robot identifier.";
    clearSelection();
    return;
  }

  // Update state
  const bool identifier_changed = (selected_robot_namespace_ != normalized_identifier);
  const bool type_changed = (use_namespace_discovery_ != is_namespace);

  if (identifier_changed || type_changed)
  {
    selected_robot_namespace_ = normalized_identifier;
    use_namespace_discovery_ = is_namespace;

    if (identifier_changed)
    {
      emit selectedRobotNamespaceChanged();
      emit selectedRobotDisplayNameChanged();
    }

    qInfo() << "[RobotManager::selectRobot] Selected robot:" << normalized_identifier << "Type:" << (is_namespace ? "namespace" : "node name");
  }
}

void RobotManager::clearSelection()
{
  const QString topic_name = buildTopicName(QString::fromUtf8(T_CMD_VEL));

  if (!selected_robot_namespace_.isEmpty())
  {
    selected_robot_namespace_.clear();
    use_namespace_discovery_ = true;
    emit selectedRobotNamespaceChanged();
    emit selectedRobotDisplayNameChanged();
  }
}

void RobotManager::setUseNamespaceDiscovery(bool use_namespace_discovery)
{
  if (use_namespace_discovery_ != use_namespace_discovery)
  {
    use_namespace_discovery_ = use_namespace_discovery;
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

  normalized.replace(" ", "_");

  return normalized;
}

QString RobotManager::buildTopicName(const QString& topic_suffix) const
{
  if (use_namespace_discovery_)
  {
    return selected_robot_namespace_ + topic_suffix;
  }

  else
  {
    return topic_suffix;
  }
}

ROBOGait::control::ManualControl* RobotManager::getManualControl() const { return manual_control_.get(); }

void RobotManager::enableManualControl()
{
  if (!manual_control_)
  {
    qCritical() << "[RobotManager::enableManualControl] ManualControl not initialized";
    return;
  }

  if (selected_robot_namespace_.isEmpty())
  {
    qCritical() << "[RobotManager::enableManualControl] No robot selected";
    return;
  }

  const QString topic_name = buildTopicName(QString::fromUtf8(T_CMD_VEL));

  manual_control_->setTopicName(topic_name);

  qInfo() << "[RobotManager::enableManualControl] Manual control enabled for topic:" << topic_name;
}

void RobotManager::disableManualControl()
{
  if (!manual_control_)
  {
    qCritical() << "[RobotManager::disableManualControl] ManualControl not initialized";
    return;
  }

  // Destroy the publisher to stop manual control
  manual_control_->destroyPublisher();

  qInfo() << "[RobotManager::disableManualControl] Manual control disabled";
}