#include <QDebug>

#include "RosNodeManager.hpp"

using namespace ROBOGait::manager;

RosNodeManager::RosNodeManager() :
    node_name_("ros_node_manager"),
    ros_node_(nullptr),
    executor_(nullptr),
    spin_thread_(),
    robot_discovery_(nullptr),
    robot_manager_(nullptr),
    is_running_(false),
    use_namespace_discovery_(true)
{
  qInfo() << "[RosNodeManager::RosNodeManager] Create RosNodeManager";
  robot_discovery_ = std::make_unique<RobotDiscovery>();
  robot_manager_ = std::make_unique<RobotManager>();

  // Synchronize initial namespace discovery state
  robot_discovery_->setUseNamespaceDiscovery(use_namespace_discovery_);
  robot_manager_->setUseNamespaceDiscovery(use_namespace_discovery_);
}

RosNodeManager::~RosNodeManager()
{
  shutdown();
  qInfo() << "[RosNodeManager::~RosNodeManager] RosNodeManager destroyed";
}

bool RosNodeManager::isRunning() const { return is_running_; }

QString RosNodeManager::getNodeName() const { return node_name_; }

void RosNodeManager::setNodeName(const QString& name)
{
  const QString trimmed_name = name.trimmed();
  if (trimmed_name.isEmpty())
  {
    qCritical() << "[RosNodeManager::setNodeName] Node name cannot be empty";
    return;
  }

  if (node_name_ != trimmed_name)
  {
    node_name_ = trimmed_name;
    emit nodeNameChanged();
  }
}

bool RosNodeManager::getUseNamespaceDiscovery() const { return use_namespace_discovery_; }

void RosNodeManager::setUseNamespaceDiscovery(bool use_namespace_discovery)
{
  if (use_namespace_discovery_ != use_namespace_discovery)
  {
    use_namespace_discovery_ = use_namespace_discovery;

    if (robot_discovery_)
    {
      robot_discovery_->setUseNamespaceDiscovery(use_namespace_discovery);
    }

    if (robot_manager_)
    {
      robot_manager_->setUseNamespaceDiscovery(use_namespace_discovery);
    }

    emit useNamespaceDiscoveryChanged();
  }
}

RobotDiscovery* RosNodeManager::getRobotDiscovery() const { return robot_discovery_.get(); }

RobotManager* RosNodeManager::getRobotManager() const { return robot_manager_.get(); }

void RosNodeManager::initialize(int argc, char** argv)
{
  if (is_running_)
  {
    qWarning() << "[RosNodeManager::initialize] Node is already running";
    return;
  }

  if (!rclcpp::ok())
  {
    qInfo() << "[RosNodeManager::initialize] Initializing ROS";
    rclcpp::init(argc, argv);
  }

  ros_node_ = std::make_shared<rclcpp::Node>(node_name_.toStdString(), "RoboGait_GUI");
  executor_ = std::make_unique<rclcpp::executors::MultiThreadedExecutor>();

  executor_->add_node(ros_node_);

  robot_discovery_->setROSNode(ros_node_.get());
  robot_manager_->setROSNode(ros_node_.get());

  is_running_ = true;

  emit isRunningChanged();
  emit rosNodeConnected(ros_node_.get());

  startSpinThread();

  qInfo() << "[RosNodeManager::initialize] ROS Node" << node_name_ << "initialized and running";
}

void RosNodeManager::shutdown()
{
  if (!is_running_)
  {
    qWarning() << "[RosNodeManager::shutdown] Node is not running";
    return;
  }

  robot_discovery_->stopScanning();
  robot_manager_->clearSelection();

  stopSpinThread();

  if (executor_ && ros_node_)
  {
    executor_->remove_node(ros_node_);
  }

  ros_node_.reset();
  executor_.reset();

  if (rclcpp::ok())
  {
    rclcpp::shutdown();
  }

  is_running_ = false;

  emit isRunningChanged();
  emit rosNodeConnected(nullptr);

  qInfo() << "[RosNodeManager::shutdown] ROS Node" << node_name_ << "shut down";
}

void RosNodeManager::startSpinThread()
{
  if (!executor_)
  {
    qCritical() << "[RosNodeManager::startSpinThread] Cannot start spin thread, executor is null";
    return;
  }

  spin_thread_ = std::thread([this]() { executor_->spin(); });

  qInfo() << "[RosNodeManager::startSpinThread] Spin thread started";
}

void RosNodeManager::stopSpinThread()
{
  if (executor_)
  {
    executor_->cancel();
  }

  if (spin_thread_.joinable())
  {
    spin_thread_.join();
  }

  qCritical() << "[RosNodeManager::stopSpinThread] Spin thread stopped";
}
