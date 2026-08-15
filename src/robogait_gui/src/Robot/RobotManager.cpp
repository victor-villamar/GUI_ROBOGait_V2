#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

#include <QDebug>
#include <QVariantMap>

#include "Map/Utils/Utils.hpp"
#include "Robot/RobotManager.hpp"
#include "Ros/Define.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"
#include "Services/RobotServiceClient.hpp"

using namespace ROBOGait::robot::manager;

RobotManager::RobotManager() :
    parent_node_(nullptr),
    selected_robot_namespace_(""),
    use_namespace_discovery_(true),
    use_topic_filter_(true),
    sub_robot_status_(nullptr),
    timer_robot_timeout_(nullptr),
    cb_group_(nullptr),
    is_monitoring_(false),
    battery_level_trunc_(0),
    battery_icon_("qrc:/qmlresources/icons/color/battery_0.png")
{
  qInfo() << "[RobotManager::RobotManager] RobotManager created";

  manual_control_ = std::make_unique<ROBOGait::robot::control::ManualControl>();
  map_visualization_manager_ = nullptr;
  robot_placement_controller_ = nullptr;
  robot_service_bridge_ = nullptr;
  person_detection_monitor_ = nullptr;
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

int RobotManager::getBatteryLevelTrunc() const { return battery_level_trunc_; }

QString RobotManager::getBatteryIcon() const { return battery_icon_; }

QVariantList RobotManager::getRobotStatusItems() const { return robot_status_items_; }

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

ROBOGait::qml::service::RobotServiceBridge* RobotManager::getRobotServiceBridge()
{
  if (!robot_service_bridge_)
  {
    robot_service_bridge_ = std::make_unique<ROBOGait::qml::service::RobotServiceBridge>();
  }
  return robot_service_bridge_.get();
}

ROBOGait::robot::RobotPlacementController* RobotManager::getRobotPlacementController()
{
  if (!robot_placement_controller_)
  {
    robot_placement_controller_ = std::make_unique<ROBOGait::robot::RobotPlacementController>();
  }

  robot_placement_controller_->setMapVisualizationManager(getMapVisualizationManager());

  return robot_placement_controller_.get();
}

ROBOGait::perception::monitor::PersonDetectionMonitor* RobotManager::getPersonDetectionMonitor()
{
  if (!person_detection_monitor_)
  {
    person_detection_monitor_ = std::make_unique<ROBOGait::perception::monitor::PersonDetectionMonitor>();
  }

  if (parent_node_)
  {
    person_detection_monitor_->setROSNode(parent_node_);
  }

  if (!selected_robot_namespace_.isEmpty())
  {
    person_detection_monitor_->setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_);
  }

  return person_detection_monitor_.get();
}

void RobotManager::setROSNode(rclcpp::Node* parent_node)
{
  if (parent_node == nullptr)
  {
    qCritical() << "[RobotManager::setROSNode] No valid ROS node provided.";
    return;
  }
  parent_node_ = parent_node;

  if (!ROBOGait::ros::service::RobotServiceClient::getInstance().initialize(parent_node_))
  {
    qCritical() << "[RobotManager::setROSNode] RobotServiceClient initialization failed";
  }

  manual_control_->setROSNode(parent_node);
  if (map_visualization_manager_)
  {
    map_visualization_manager_->setROSNode(parent_node);
  }

  if (person_detection_monitor_)
  {
    person_detection_monitor_->setROSNode(parent_node);
  }

  cb_group_ = parent_node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  timer_robot_timeout_ =
      parent_node_->create_wall_timer(ROBOGait::ros::define::TIME_TO_ROBOT_TIMEOUT_MS, std::bind(&RobotManager::callbackRobotTimeoutTimer, this),
                                      cb_group_); // one-shot=false, auto-start=false
  timer_robot_timeout_->cancel();                 // Disable auto-start

  qDebug() << "[RobotManager::setROSNode] ROS node set successfully";
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

    qDebug() << "[RobotManager::selectRobot] Selected robot:" << normalized_identifier << "Type:" << (is_namespace ? "namespace" : "node name");

    // Update map visualization for new robot
    if (map_visualization_manager_)
    {
      map_visualization_manager_->setSelectedRobot(normalized_identifier, is_namespace);
    }

    ROBOGait::context::RobotContext context;
    if (context.setSelectedRobot(normalized_identifier, is_namespace))
    {
      ROBOGait::ros::service::RobotServiceClient::getInstance().setRobotContext(context);
      if (person_detection_monitor_)
      {
        person_detection_monitor_->setSelectedRobot(normalized_identifier, is_namespace);
      }
    }
    else
    {
      ROBOGait::ros::service::RobotServiceClient::getInstance().clearRobotContext();
      if (person_detection_monitor_)
      {
        person_detection_monitor_->stopMonitoring();
      }
    }

    if (use_topic_filter_)
    {
      startMonitoring();
    }
  }
}

void RobotManager::clearSelection()
{
  pub_pose_initialize_.reset();

  if (use_topic_filter_)
  {
    stopMonitoring();
  }

  if (person_detection_monitor_)
  {
    person_detection_monitor_->stopMonitoring();
  }

  if (!selected_robot_namespace_.isEmpty())
  {
    selected_robot_namespace_.clear();
    use_namespace_discovery_ = true;
    emit selectedRobotNamespaceChanged();
    emit selectedRobotDisplayNameChanged();

    ROBOGait::ros::service::RobotServiceClient::getInstance().clearRobotContext();

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

  const QString topic_name = buildTopicName(QString::fromStdString(ROBOGait::ros::topics::T_CMD_VEL));

  manual_control_->setTopicName(topic_name);

  qDebug() << "[RobotManager::enableManualControl] Manual control enabled for topic:" << topic_name;
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

  qDebug() << "[RobotManager::disableManualControl] Manual control disabled";
}

void RobotManager::publishInitialPose(double x, double y, double theta)
{
  if (!parent_node_)
  {
    qCritical() << "[RobotManager::publishInitialPose] Parent node is not set";
    return;
  }

  if (selected_robot_namespace_.isEmpty())
  {
    qCritical() << "[RobotManager::publishInitialPose] No robot selected";
    return;
  }

  if (!pub_pose_initialize_)
  {
    const QString topic_name = buildTopicName(QString::fromStdString(ROBOGait::ros::topics::T_POSE_INITIALIZE));
    pub_pose_initialize_ = parent_node_->create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(topic_name.toStdString(),
                                                                                                         ROBOGait::ros::QosProfiles::QOS_RELIABLE_LATCH());
  }

  auto msg = geometry_msgs::msg::PoseWithCovarianceStamped();

  QString frame_id = buildTopicName("/" + QString::fromStdString(ROBOGait::ros::topics::TF_MAP_FRAME));

  msg.header.frame_id = frame_id.toStdString();
  msg.header.stamp = parent_node_->now();

  msg.pose.pose.position.x = x;
  msg.pose.pose.position.y = y;
  msg.pose.pose.position.z = 0.0;
  msg.pose.pose.orientation = ROBOGait::map::utils::createQuaternionFromYaw(theta);

  msg.pose.covariance.fill(0.0);
  msg.pose.covariance[0] = 0.05;  // Variance in x (50cm)
  msg.pose.covariance[7] = 0.05;  // Variance in y (50cm)
  msg.pose.covariance[35] = 0.15; // Variance in yaw (15 degrees)

  pub_pose_initialize_->publish(std::move(msg));
}

QString RobotManager::getMapPreviewPath(const QString& mapName) const { return ROBOGait::map::utils::getMapPreviewPath(mapName); }

bool RobotManager::deleteMapPreview(const QString& mapName) const { return ROBOGait::map::utils::deleteMapPreview(mapName); }

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

void RobotManager::callbackRobotStatus(const command_executor_msgs::msg::RobotStatus::SharedPtr msg)
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

  if (!msg)
  {
    qWarning() << "[RobotManager::callbackRobotStatus] Received null message, ignoring";
    return;
  }

  last_robot_message_time_ = parent_node_->now();

  robot_status_info_.id = msg->id;
  robot_status_info_.ns = QString::fromStdString(msg->ns);
  robot_status_info_.version = QString::fromStdString(msg->version);
  robot_status_info_.hardware_id = msg->hardware_id;
  robot_status_info_.serial_number = QString::fromStdString(msg->serial_number);

  const float clamped_battery = std::max(0.0f, std::min(100.0f, msg->battery));
  const int bucket = static_cast<int>(std::floor(clamped_battery / 5.0f)) * 5;
  battery_level_trunc_ = std::max(0, std::min(100, bucket));
  battery_icon_ = QString("qrc:/qmlresources/icons/color/battery_%1.png").arg(battery_level_trunc_);

  robot_status_items_ = buildStatusItems(robot_status_info_);

  emit robotStatusChanged();
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

  ROBOGait::context::RobotContext context;
  if (!context.setSelectedRobot(selected_robot_namespace_, use_namespace_discovery_))
  {
    qCritical() << "[RobotManager::startMonitoring] Cannot start monitoring: Invalid robot context";
    return;
  }

  const std::string full_topic = context.resolveTopic(ROBOGait::ros::topics::T_ROBOT_STATUS);

  qDebug() << "[RobotManager::startMonitoring] Starting monitoring for:" << full_topic.c_str()
           << "(mode:" << (use_namespace_discovery_ ? "namespace" : "node name") << ")";

  sub_robot_status_ = parent_node_->create_subscription<command_executor_msgs::msg::RobotStatus>(
      full_topic, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT(), std::bind(&RobotManager::callbackRobotStatus, this, std::placeholders::_1));

  last_robot_message_time_ = parent_node_->now();

  is_monitoring_ = true;
  timer_robot_timeout_->reset();
}

void RobotManager::stopMonitoring()
{
  if (!is_monitoring_)
  {
    qDebug() << "[RobotManager::stopMonitoring] Monitoring is not active, nothing to stop";
    return;
  }

  if (timer_robot_timeout_)
  {
    timer_robot_timeout_->cancel();
  }

  // Destroy subscription
  sub_robot_status_.reset();

  is_monitoring_ = false;

  qDebug() << "[RobotManager::stopMonitoring] Monitoring stopped";
}

void RobotManager::addStatusItem(QVariantList& items, const QString& label, const QString& value) const
{
  QVariantMap row;
  row.insert("label", label);
  row.insert("value", value);
  items.append(row);
}

QVariantList RobotManager::buildStatusItems(const ROBOGait::robot::manager::RobotManager::RobotStatusInfo& info) const
{
  QVariantList items;
  addStatusItem(items, "ID", QString::number(static_cast<int>(info.id)));
  addStatusItem(items, "Namespace", info.ns);
  addStatusItem(items, "Version", info.version);
  addStatusItem(items, "Hardware ID", QString::number(static_cast<qulonglong>(info.hardware_id)));
  addStatusItem(items, "S/N", info.serial_number);

  return items;
}