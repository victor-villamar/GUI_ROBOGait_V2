#include <QDebug>
#include <chrono>
#include <cmath>

#include "Robot/RobotManager.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::robot::manager;

RobotManager::RobotManager() :
    parent_node_(nullptr),
    selected_robot_namespace_(""),
    use_namespace_discovery_(true),
    use_topic_filter_(true),
    sub_robot_status_(nullptr),
    timer_robot_timeout_(nullptr),
    cb_group_(nullptr),
    is_monitoring_(false)
{
  qInfo() << "[RobotManager::RobotManager] RobotManager created";

  manual_control_ = std::make_unique<ROBOGait::robot::control::ManualControl>();
  map_visualization_manager_ = nullptr;
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
  if (map_visualization_manager_)
  {
    map_visualization_manager_->setROSNode(parent_node);
  }

  cb_group_ = parent_node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  timer_robot_timeout_ =
      parent_node_->create_wall_timer(std::chrono::milliseconds(TIME_TO_ROBOT_TIMEOUT), std::bind(&RobotManager::callbackRobotTimeoutTimer, this),
                                      cb_group_); // one-shot=false, auto-start=false
  timer_robot_timeout_->cancel();                 // Disable auto-start

  qInfo() << "[RobotManager::setROSNode] ROS node set successfully";
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
    if (use_topic_filter_)
    {
      stopMonitoring();
    }

    selected_robot_namespace_ = normalized_identifier;
    use_namespace_discovery_ = is_namespace;

    if (identifier_changed)
    {
      emit selectedRobotNamespaceChanged();
      emit selectedRobotDisplayNameChanged();
    }

    qInfo() << "[RobotManager::selectRobot] Selected robot:" << normalized_identifier << "Type:" << (is_namespace ? "namespace" : "node name");

    // Update map visualization for new robot
    if (map_visualization_manager_)
    {
      map_visualization_manager_->setSelectedRobot(normalized_identifier, is_namespace);
    }

    if (use_topic_filter_)
    {
      startMonitoring();
    }
  }
}

void RobotManager::clearSelection()
{
  const QString topic_name = buildTopicName(QString::fromUtf8(T_CMD_VEL));

  if (use_topic_filter_)
  {
    stopMonitoring();
  }

  if (!selected_robot_namespace_.isEmpty())
  {
    selected_robot_namespace_.clear();
    use_namespace_discovery_ = true;
    emit selectedRobotNamespaceChanged();
    emit selectedRobotDisplayNameChanged();

    // Destroy map visualization subscriptions
    if (map_visualization_manager_)
    {
      map_visualization_manager_->destroySubscriptions();
    }
  }
}

void RobotManager::setUseNamespaceDiscovery(bool use_namespace_discovery)
{
  if (use_namespace_discovery_ != use_namespace_discovery)
  {
    use_namespace_discovery_ = use_namespace_discovery;
  }
}

bool RobotManager::getUseNamespaceDiscovery() const { return use_namespace_discovery_; }

void RobotManager::setUseTopicFilter(bool use_topic_filter)
{
  if (use_topic_filter_ != use_topic_filter)
  {
    use_topic_filter_ = use_topic_filter;

    if (is_monitoring_)
    {
      stopMonitoring();
      if (use_topic_filter_)
      {
        startMonitoring();
      }
    }
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

ROBOGait::robot::control::ManualControl* RobotManager::getManualControl() const { return manual_control_.get(); }

ROBOGait::map::manager::MapVisualizationManager* RobotManager::getMapVisualizationManager()
{
  if (!map_visualization_manager_)
  {
    map_visualization_manager_ = std::make_unique<ROBOGait::map::manager::MapVisualizationManager>();

    if (parent_node_)
    {
      map_visualization_manager_->setROSNode(parent_node_);
      if (!selected_robot_namespace_.isEmpty())
      {
        map_visualization_manager_->setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_);
      }
    }
  }

  return map_visualization_manager_.get();
}

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

void RobotManager::startMonitoring()
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotManager::startMonitoring] Cannot start monitoring: null parent node";
    return;
  }

  if (selected_robot_namespace_.isEmpty())
  {
    qCritical() << "[RobotManager::startMonitoring] Cannot start monitoring: No robot selected";
    return;
  }

  std::string full_topic;
  if (use_namespace_discovery_)
  {
    full_topic = selected_robot_namespace_.toStdString() + T_ROBOT_STATUS;
  }
  else
  {
    full_topic = T_ROBOT_STATUS;
  }

  qInfo() << "[RobotManager::startMonitoring] Starting monitoring for:" << full_topic.c_str()
          << "(mode:" << (use_namespace_discovery_ ? "namespace" : "node name") << ")";

  sub_robot_status_ = parent_node_->create_subscription<std_msgs::msg::String>(full_topic, QOS_BEST_EFFORT,
                                                                               std::bind(&RobotManager::callbackRobotStatus, this, std::placeholders::_1));

  last_robot_message_time_ = parent_node_->now();

  is_monitoring_ = true;
  timer_robot_timeout_->reset();

  qInfo() << "[RobotManager::startMonitoring] Monitoring started successfully";
}

void RobotManager::stopMonitoring()
{
  if (!is_monitoring_)
  {
    qInfo() << "[RobotManager::stopMonitoring] Monitoring is not active, nothing to stop";
    return;
  }

  qInfo() << "[RobotManager::stopMonitoring] Stopping monitoring";

  if (timer_robot_timeout_)
  {
    timer_robot_timeout_->cancel();
  }

  // Destroy subscription
  sub_robot_status_.reset();

  is_monitoring_ = false;

  qInfo() << "[RobotManager::stopMonitoring] Monitoring stopped";
}

void RobotManager::checkRobotAvailability(const QStringList& available_robots)
{
  if (use_topic_filter_)
  {
    return;
  }

  if (selected_robot_namespace_.isEmpty())
  {
    qWarning() << "[RobotManager::checkRobotAvailability] No robot selected";
    return;
  }

  if (!available_robots.contains(selected_robot_namespace_))
  {
    qWarning() << "[RobotManager::checkRobotAvailability] Robot" << selected_robot_namespace_ << "is no longer available in the graph";

    emit robotDisconnected();
  }
}

void RobotManager::callbackRobotStatus(const std_msgs::msg::String::SharedPtr msg)
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotManager::callbackRobotStatus] Received message but parent node is null, ignoring";
    return;
  }

  if (!is_monitoring_)
  {
    qCritical() << "[RobotManager::callbackRobotStatus] Received message while monitoring is inactive, ignoring";
    return;
  }

  last_robot_message_time_ = parent_node_->now();
}

void RobotManager::callbackRobotTimeoutTimer()
{
  if (parent_node_ == nullptr)
  {
    qCritical() << "[RobotManager::callbackRobotTimeoutTimer] Cannot check timeout: null parent node";
    return;
  }

  if (!is_monitoring_)
  {
    qCritical() << "[RobotManager::callbackRobotTimeoutTimer] Cannot check timeout: monitoring is inactive";
    return;
  }

  const auto now = parent_node_->now();
  const auto elapsed = (now - last_robot_message_time_).seconds();

  if (elapsed > TIMEOUT_SECONDS)
  {
    qWarning() << "[RobotManager::callbackRobotTimeoutTimer] Robot timeout detected!"
               << "Robot:" << selected_robot_namespace_ << "has disconnected after" << elapsed << "seconds";

    stopMonitoring();

    emit robotDisconnected();
  }
}