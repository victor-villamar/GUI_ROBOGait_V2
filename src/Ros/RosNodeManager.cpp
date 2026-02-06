#include <QDebug>
#include <rclcpp/executor_options.hpp>
#include <rclcpp/init_options.hpp>
#include <rclcpp/node_options.hpp>

#include "Ros/RosNodeManager.hpp"

using namespace ROBOGait::ros::manager;

RosNodeManager::RosNodeManager() :
    node_name_("ros_node_manager"),
    context_(nullptr),
    ros_node_(nullptr),
    executor_(nullptr),
    spin_thread_(),
    robot_discovery_(nullptr),
    robot_manager_(nullptr),
    is_running_(false),
    use_namespace_discovery_(true),
    current_domain_id_(0)
{
  qInfo() << "[RosNodeManager::RosNodeManager] Create RosNodeManager";
  robot_discovery_ = std::make_unique<ROBOGait::robot::discovery::RobotDiscovery>();
  robot_manager_ = std::make_unique<ROBOGait::robot::manager::RobotManager>();

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

ROBOGait::robot::discovery::RobotDiscovery* RosNodeManager::getRobotDiscovery() const { return robot_discovery_.get(); }

ROBOGait::robot::manager::RobotManager* RosNodeManager::getRobotManager() const { return robot_manager_.get(); }

void RosNodeManager::initialize(int argc, char** argv, int domain_id)
{
  if (is_running_)
  {
    qWarning() << "[RosNodeManager::initialize] Node is already running";
    return;
  }

  current_domain_id_ = domain_id;

  rclcpp::InitOptions init_options;
  init_options.set_domain_id(static_cast<size_t>(domain_id));

  context_ = std::make_shared<rclcpp::Context>();
  context_->init(argc, argv, init_options);

  if (!context_->is_valid())
  {
    qCritical() << "[RosNodeManager::initialize] Failed to create valid ROS context";
    return;
  }

  rclcpp::NodeOptions node_options;
  node_options.context(context_);

  ros_node_ = std::make_shared<rclcpp::Node>(node_name_.toStdString(), "RoboGait_GUI", node_options);

  rclcpp::ExecutorOptions executor_options;
  executor_options.context = context_;
  executor_ = std::make_unique<rclcpp::executors::MultiThreadedExecutor>(executor_options);

  executor_->add_node(ros_node_);

  robot_discovery_->setROSNode(ros_node_.get());
  robot_manager_->setROSNode(ros_node_.get());

  is_running_ = true;

  emit isRunningChanged();
  emit rosNodeConnected(ros_node_.get());

  startSpinThread();

  qCritical() << "[RosNodeManager::initialize] ROS Node" << node_name_ << "initialized and running on domain" << domain_id;
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

  if (context_ && context_->is_valid())
  {
    qInfo() << "[RosNodeManager::shutdown] Shutting down ROS context from domain" << current_domain_id_;

    if (context_->shutdown("Application requested shutdown"))
    {
      qInfo() << "[RosNodeManager::shutdown] Context shutdown successful";
    }
    else
    {
      qWarning() << "[RosNodeManager::shutdown] Context was already shut down";
    }
  }
  else
  {
    qInfo() << "[RosNodeManager::shutdown] Context is null or invalid, skipping shutdown";
  }

  context_.reset();

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
