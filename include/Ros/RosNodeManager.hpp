#pragma once

#include <QObject>
#include <atomic>
#include <memory>
#include <rclcpp/context.hpp>
#include <rclcpp/executors.hpp>
#include <rclcpp/node.hpp>
#include <thread>

#include "Robot/RobotDiscovery.hpp"
#include "Robot/RobotManager.hpp"

namespace ROBOGait
{
namespace ros
{
namespace manager
{

/**
 * @brief Class for managing the ROS node
 */
class RosNodeManager : public QObject
{
  Q_OBJECT
public:
  /**
   * @brief Constructor of RosNodeManager class
   */
  explicit RosNodeManager();

  /**
   * @brief Destructor of RosNodeManager class
   */
  ~RosNodeManager() override;

  // clang-format off
  Q_PROPERTY(bool isRunning
             READ isRunning
             NOTIFY isRunningChanged
  )
  Q_PROPERTY(QString nodeName
             READ getNodeName
             WRITE setNodeName
             NOTIFY nodeNameChanged
  )
  Q_PROPERTY(bool useNamespaceDiscovery
             READ getUseNamespaceDiscovery
             WRITE setUseNamespaceDiscovery
             NOTIFY useNamespaceDiscoveryChanged
  )
  Q_PROPERTY(ROBOGait::robot::discovery::RobotDiscovery* robotDiscovery
             READ getRobotDiscovery
             CONSTANT
  )
  Q_PROPERTY(ROBOGait::robot::manager::RobotManager* robotManager
             READ getRobotManager
             CONSTANT
  )
  // clang-format on

  /**
   * @brief Checks if the ROS node is running
   * @return True if the node is running, false otherwise
   */
  bool isRunning() const;

  /**
   * @brief Gets the name of the ROS node
   * @return The name of the ROS node
   */
  QString getNodeName() const;

  /**
   * @brief Sets the name of the ROS node
   * @param name The name to set for the ROS node
   */
  void setNodeName(const QString& name);

  /**
   * @brief Establish if namespace discovery is enabled
   * @return True if namespace discovery is enabled, false otherwise
   */
  bool getUseNamespaceDiscovery() const;

  /**
   * @brief Sets the namespace discovery flag
   * @param use_namespace_discovery True to enable namespace discovery, false to disable
   */
  void setUseNamespaceDiscovery(bool use_namespace_discovery);

  /**
   * @brief Sets the topic filter flag for robot discovery and monitoring
   * @param use_topic_filter True to enable topic filtering, false to disable
   */
  void setUseTopicFilter(bool use_topic_filter);

  /**
   * @brief Gets the RobotDiscovery instance
   * @return A pointer to the RobotDiscovery instance
   */
  ROBOGait::robot::discovery::RobotDiscovery* getRobotDiscovery() const;

  /**
   * @brief Gets the RobotManager instance
   * @return A pointer to the RobotManager instance
   */
  ROBOGait::robot::manager::RobotManager* getRobotManager() const;

  /**
   * @brief Initializes the ROS node manager
   * @param argc The argument count
   * @param argv The argument vector
   * @param domain_id ROS domain ID to use for the node (0-232)
   */
  void initialize(int argc, char** argv, uint32_t domain_id = 0);

  /**
   * @brief Restarts ROS node with a new domain ID
   * @param new_domain_id New ROS domain ID (0-232)
   * @param argc The argument count
   * @param argv The argument vector
   * @return True if restart was successful, false otherwise
   */
  bool restartWithDomain(uint32_t new_domain_id, int argc, char** argv);

  /**
   * @brief Shuts down the ROS node manager
   */
  Q_INVOKABLE void shutdown();

signals:
  void isRunningChanged();
  void nodeNameChanged();
  void useNamespaceDiscoveryChanged();
  void rosNodeConnected(rclcpp::Node* node);

private:
  /**
   * @brief Starts the spin thread for the ROS node
   */
  void startSpinThread();

  /**
   * @brief Stops the spin thread for the ROS node
   */
  void stopSpinThread();

  QString node_name_;                                                  /**< The name of the ROS node */
  std::shared_ptr<rclcpp::Context> context_;                           /**< ROS context with domain ID configuration */
  std::shared_ptr<rclcpp::Node> ros_node_;                             /**< The ROS node instance */
  std::unique_ptr<rclcpp::executors::MultiThreadedExecutor> executor_; /**< The executor for the ROS node */
  std::thread spin_thread_;                                            /**< The thread for spinning the ROS node */

  std::unique_ptr<ROBOGait::robot::discovery::RobotDiscovery> robot_discovery_; /**< The RobotDiscovery instance */
  std::unique_ptr<ROBOGait::robot::manager::RobotManager> robot_manager_;       /**< The RobotManager instance */

  std::atomic<bool> is_running_; /**< Indicates if the ROS node is running */
  bool use_namespace_discovery_; /**< Indicates if namespace discovery is enabled */
  uint32_t current_domain_id_;   /**< Current ROS domain ID */
};
} // namespace manager
} // namespace ros
} // namespace ROBOGait