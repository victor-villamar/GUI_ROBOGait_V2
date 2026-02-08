#pragma once

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <rclcpp/node.hpp>
#include <string>
#include <vector>

namespace ROBOGait
{
namespace robot
{
namespace discovery
{
/**
 * @brief Class for discovering robots in the ROS network
 */
class RobotDiscovery : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Enumeration for the different states of the discovery process
   */
  enum class State
  {
    IDLE,
    NO_NODE,
    SCANNING,
    NO_ROBOTS,
    ROBOTS_FOUND,
    ERROR
  };
  Q_ENUM(State)

  /**
   * @brief Constructor of RobotDiscovery class
   */
  RobotDiscovery();

  /**
   * @brief Destructor of RobotDiscovery class
   */
  ~RobotDiscovery() override = default;

  // clang-format off
  Q_PROPERTY(QStringList robotsNamespaces
             READ getRobotsNamespaces
             NOTIFY robotsNamespacesChanged
  )
  Q_PROPERTY(bool isScanning
             READ isScanning
             NOTIFY isScanningChanged
  )
  Q_PROPERTY(State state
             READ getState
             NOTIFY stateChanged
  )
  Q_PROPERTY(int pollInterval
             READ getPollInterval
             WRITE setPollInterval
             NOTIFY pollIntervalChanged
  )
  // clang-format on

  /**
   * @brief Get the list of discovered robot namespaces
   *
   * @return List of discovered robot namespaces
   */
  QStringList getRobotsNamespaces() const;

  /**
   * @brief Check if the discovery process is currently scanning for robots
   *
   * @return True if scanning, false otherwise
   */
  bool isScanning() const;

  /**
   * @brief Get the current state of the discovery process
   *
   * @return Current state of the discovery process
   */
  State getState() const;

  /**
   * @brief Get the current polling interval
   *
   * @return Current polling interval in milliseconds
   */
  int getPollInterval() const;

  /**
   * @brief Set the polling interval
   *
   * @param interval New polling interval in milliseconds
   */
  void setPollInterval(int interval);

  /**
   * @brief Set the ROS node for the discovery process
   *
   * @param node Pointer to the ROS node
   */
  void setROSNode(rclcpp::Node* node);

  /**
   * @brief Start the robot discovery process
   */
  Q_INVOKABLE void startScanning();

  /**
   * @brief Stop the robot discovery process
   */
  Q_INVOKABLE void stopScanning();

  /**
   * @brief Set whether to use namespace-based discovery
   *
   * @param use_namespace_discovery True to discover by namespace, false to discover by node names
   */
  void setUseNamespaceDiscovery(bool use_namespace_discovery);

signals:
  void isScanningChanged();
  void stateChanged();
  void pollIntervalChanged();
  void robotsNamespacesChanged();

private:
  /**
   * @brief Set the list of discovered robots namespaces
   *
   * @param displayRobots List of robot names to display
   * @param robot_namespaces List of robot namespaces
   */
  void setRobotsNamespaces(const QStringList& robot_namespaces);

  /**
   * @brief Set the scanning state
   *
   * @param is_scanning True if scanning, false otherwise
   */
  void setIsScanning(bool is_scanning);

  /**
   * @brief Set the state of the discovery process
   *
   * @param state New state of the discovery process
   */
  void setState(State state);

  /**
   * @brief Update the discovery process from the ROS graph
   */
  void updateFromGraph();

  /**
   * @brief Compute the list of robots from the ROS graph
   *
   * @param nodes_names_and_namespaces List of node names and namespaces
   * @param self_node_name Name of the self node
   * @return List containing the robot display namespaces
   */
  QStringList computeRobotsListFromGraph(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces, const std::string& self_node_name);

  /**
   * @brief Build the list of robot namespaces from the given namespaces
   *
   * @param namespaces Set of robot namespaces
   * @return List of robot display namespaces
   */
  static QStringList buildRobotNamespacesFromNamespaces(const QSet<QString>& namespaces);

  /**
   * @brief Build the list of robot node names when not using namespace discovery
   *
   * @param nodes_names_and_namespaces List of node names and namespaces
   * @param self_node_name Name of the self node to exclude
   * @return List of robot node names
   */
  static QStringList buildRobotNodeNamesFromGraph(const std::vector<std::pair<std::string, std::string>>& nodes_names_and_namespaces,
                                                  const std::string& self_node_name);

  /**
   * @brief Check if a namespace has the robot_status topic
   *
   * @param robot_namespace Namespace of the robot to verify
   * @return true if the topic exists, false otherwise
   */
  bool hasRobotStatusTopic(const std::string& robot_namespace) const;

  rclcpp::Node* parent_node_;     /**< Pointer to the ROS node */
  QStringList robots_namespaces_; /**< List of discovered robot namespaces or node names */
  bool is_scanning_;              /**< Flag indicating if scanning is active */
  State state_;                   /**< Current state of the discovery process */
  int poll_interval_;             /**< Polling interval in milliseconds */
  QTimer poll_timer_;             /**< Timer for polling */
  bool use_namespace_discovery_;  /**< True to discover by namespace, false to discover by node names */
};
} // namespace discovery
} // namespace robot
} // namespace ROBOGait