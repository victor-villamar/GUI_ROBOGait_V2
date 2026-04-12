#pragma once

#include <cstdint>
#include <memory>

#include <QObject>
#include <QString>
#include <QVariantList>

#include <rclcpp/callback_group.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/time.hpp>
#include <rclcpp/timer.hpp>

#include <command_executor_msgs/msg/robot_status.hpp>
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>

#include "Map/MapVisualizationManager.hpp"
#include "Robot/ManualControl.hpp"
#include "Robot/RobotPlacementController.hpp"
#include "Services/RobotServiceBridge.hpp"

namespace ROBOGait
{
namespace robot
{
namespace manager
{
/**
 * @brief Manages the robot's state and behavior
 */
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
             NOTIFY selectedRobotNamespaceChanged)

  Q_PROPERTY(QString selectedRobotDisplayName
             READ getSelectedRobotDisplayName
             NOTIFY selectedRobotDisplayNameChanged)

  Q_PROPERTY(int batteryLevelTrunc
             READ getBatteryLevelTrunc
             NOTIFY robotStatusChanged)

  Q_PROPERTY(QString batteryIcon
             READ getBatteryIcon
             NOTIFY robotStatusChanged)

  Q_PROPERTY(QVariantList robotStatusItems
             READ getRobotStatusItems
             NOTIFY robotStatusChanged)

  Q_PROPERTY(ROBOGait::robot::control::ManualControl* manualControl
             READ getManualControl
             CONSTANT)

  Q_PROPERTY(ROBOGait::map::manager::MapVisualizationManager* mapVisualizationManager
             READ getMapVisualizationManager
             CONSTANT)

  Q_PROPERTY(ROBOGait::qml::service::RobotServiceBridge* robotServiceBridge
             READ getRobotServiceBridge
             CONSTANT)

  Q_PROPERTY(ROBOGait::robot::RobotPlacementController* robotPlacementController
             READ getRobotPlacementController
             CONSTANT)
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
   * @brief Get battery level truncated to 5% steps
   */
  int getBatteryLevelTrunc() const;

  /**
   * @brief Get battery icon path for the truncated level
   */
  QString getBatteryIcon() const;

  /**
   * @brief Get robot status fields as a list of label/value pairs
   */
  QVariantList getRobotStatusItems() const;

  /**
   * @brief Get the manual control instance
   */
  ROBOGait::robot::control::ManualControl* getManualControl() const;

  /**
   * @brief Get the map visualization manager instance
   *
   * Creates the manager on first access
   */
  ROBOGait::map::manager::MapVisualizationManager* getMapVisualizationManager();

  /**
   * @brief Get the command executor bridge instance
   *
   * Creates the bridge on first access
   */
  ROBOGait::qml::service::RobotServiceBridge* getRobotServiceBridge();

  /**
   * @brief Get the robot placement controller instance
   *
   * Creates the controller on first access
   */
  ROBOGait::robot::RobotPlacementController* getRobotPlacementController();

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
   * @brief Get whether namespace-based topic construction is enabled
   *
   * @return True if using namespace, false if using node name
   */
  bool getUseNamespaceDiscovery() const;

  /**
   * @brief Set whether to use topic filter for monitoring
   *
   * @param use_topic_filter True to monitor via topic, false to skip monitoring
   */
  void setUseTopicFilter(bool use_topic_filter);

  /**
   * @brief Check if selected robot is still available in discovery list
   *
   * @param available_robots List of currently available robot namespaces
   */
  void checkRobotAvailability(const QStringList& available_robots);

  /**
   * @brief Enables manual control mode for the selected robot
   */
  Q_INVOKABLE void enableManualControl();

  /**
   * @brief Disables manual control mode
   */
  Q_INVOKABLE void disableManualControl();

  /**
   * @brief Publish an initial pose for the robot
   *
   * @param x X coordinate of the pose
   * @param y Y coordinate of the pose
   * @param theta Orientation of the pose in radians
   */
  Q_INVOKABLE void publishInitialPose(double x, double y, double theta);

signals:
  void selectedRobotNamespaceChanged();   /**< Emitted when the selected robot namespace changes */
  void selectedRobotDisplayNameChanged(); /**< Emitted when the selected robot display name changes */
  void robotDisconnected();               /**< Emitted when robot is disconnected due to timeout */
  void robotStatusChanged();              /**< Emitted when robot status fields are updated */

private:
  /**
   * @brief Struct to hold robot status information
   *
   * @param id Robot ID
   * @param ns Robot namespace
   * @param version Robot version
   * @param hardware_id Robot hardware ID
   * @param serial_number Robot serial number
   */
  struct RobotStatusInfo
  {
    uint8_t id;
    QString ns;
    QString version;
    uint32_t hardware_id;
    QString serial_number;
  };

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

  /**
   * @brief Callback for receiving robot_status messages
   *
   * @param msg Message received from robot
   */
  void callbackRobotStatus(const command_executor_msgs::msg::RobotStatus::SharedPtr msg);

  /**
   * @brief Check robot timeout and emit signal if disconnected
   */
  void callbackRobotTimeoutTimer();

  /**
   * @brief Start monitoring the selected robot
   */
  void startMonitoring();

  /**
   * @brief Stop monitoring the robot
   */
  void stopMonitoring();

  /**
   * @brief Append a label/value pair to the list
   */
  void addStatusItem(QVariantList& items, const QString& label, const QString& value) const;

  /**
   * @brief Build a list of label/value pairs for QML
   */
  QVariantList buildStatusItems(const RobotStatusInfo& info) const;

  rclcpp::Node* parent_node_; /**< Pointer to the parent ROS node */

  std::unique_ptr<ROBOGait::robot::control::ManualControl> manual_control_;                    /**< Manual control instance */
  std::unique_ptr<ROBOGait::map::manager::MapVisualizationManager> map_visualization_manager_; /**< Map visualization manager */
  std::unique_ptr<ROBOGait::qml::service::RobotServiceBridge> robot_service_bridge_;           /**< Robot service bridge instance */
  std::unique_ptr<ROBOGait::robot::RobotPlacementController> robot_placement_controller_;      /**< Robot placement controller instance */

  QString selected_robot_namespace_; /**< The namespace of the selected robot */
  bool use_namespace_discovery_;     /**< True if selected robot is identified by namespace, false if by node name */
  bool use_topic_filter_;            /**< True to monitor via topic, false to skip monitoring */

  rclcpp::Subscription<command_executor_msgs::msg::RobotStatus>::SharedPtr sub_robot_status_;       /**< Subscriber to robot_status topic */
  rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr pub_pose_initialize_; /**< Publisher for initializing robot pose */
  rclcpp::TimerBase::SharedPtr timer_robot_timeout_;                                                /**< Timer for robot disconnection */
  rclcpp::CallbackGroup::SharedPtr cb_group_;                                                       /**< Callback group for subscriptions */
  rclcpp::Time last_robot_message_time_;                                                            /**< Timestamp of last robot message */

  bool is_monitoring_;                /**< Flag indicating if monitoring is active */
  RobotStatusInfo robot_status_info_; /**< Latest robot status information */
  int battery_level_trunc_;           /**< Battery level truncated to 5% steps */
  QString battery_icon_;              /**< Battery icon path */
  QVariantList robot_status_items_;   /**< Status items for QML */

  static constexpr double TIMEOUT_SECONDS = 3.0; /**< Timeout in seconds for robot disconnection */
};
} // namespace manager
} // namespace robot
} // namespace ROBOGait
