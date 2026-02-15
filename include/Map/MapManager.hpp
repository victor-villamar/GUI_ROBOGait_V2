#pragma once

#include <QImage>
#include <QObject>
#include <QPoint>
#include <map_msgs/msg/occupancy_grid_update.hpp>
#include <memory>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>
#include <string>

#include "Map/MapData.hpp"
#include "Map/RobotPose.hpp"

namespace ROBOGait
{
namespace map
{
namespace manager
{

/**
 * @brief Singleton manager for map visualization and robot pose tracking
 *
 */
class MapManager : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Get singleton instance
   *
   * @return Reference to the unique MapManager instance
   */
  static MapManager& getInstance();

  /**
   * @brief Delete copy constructor and assignment operator
   */
  MapManager(const MapManager&) = delete;
  MapManager& operator=(const MapManager&) = delete;

  // clang-format off
  Q_PROPERTY(bool mapAvailable
             READ isMapAvailable
             NOTIFY mapAvailableChanged)

  Q_PROPERTY(QImage mapImage
             READ getMapImage
             NOTIFY mapImageChanged)

  Q_PROPERTY(int mapWidth
             READ getMapWidth
             NOTIFY mapMetadataChanged)
  
  Q_PROPERTY(int mapHeight
             READ getMapHeight
             NOTIFY mapMetadataChanged)
  
  Q_PROPERTY(double mapResolution
             READ getMapResolution
             NOTIFY mapMetadataChanged)
  
  Q_PROPERTY(bool robotPoseAvailable
             READ isRobotPoseAvailable
             NOTIFY robotPoseChanged)

  Q_PROPERTY(QPointF robotScreenPos
             READ getRobotScreenPos
             NOTIFY robotPoseChanged)

  Q_PROPERTY(double robotScreenRotation
             READ getRobotScreenRotation
             NOTIFY robotPoseChanged)
  // clang-format on

  /**
   * @brief Set ROS node for Map Manager
   *
   * @param parent_node Pointer to the parent ROS node
   */
  void setROSNode(rclcpp::Node* parent_node);

  /**
   * @brief Set selected robot for map visualization
   *
   * @param robot_identifier Robot identifier
   * @param is_namespace If true, identifier is a namespace; if false, it's a node name
   */
  void setSelectedRobot(const QString& robot_identifier, bool is_namespace);

  /**
   * @brief Check if manager is initialized
   *
   * @return true if initialize, false otherwise
   */
  bool isInitialized() const;

  /**
   * @brief Check if map data is available
   *
   * @return true if map has been received and processed, false otherwise
   */
  bool isMapAvailable() const;

  /**
   * @brief Get the current map image for rendering
   *
   * @return QImage representing the occupancy grid map, or null image if not available
   */
  QImage getMapImage();

  /**
   * @brief Get map width
   *
   * @return Map width in pixels, or zero if map not available
   */
  int getMapWidth() const;

  /**
   * @brief Get map height in pixels
   *
   * @return Map height in pixels, or zero if map not available
   */
  int getMapHeight() const;

  /**
   * @brief Get map resolution in meters per pixel
   *
   * @return Map resolution in meters per pixel, or zero if map not available
   */
  double getMapResolution() const;

  /**
   * @brief Check if robot pose data is available
   *
   * @return true if robot pose has been received and processed, false otherwise
   */
  bool isRobotPoseAvailable() const;

  /**
   * @brief Get robot position in screen coordinates
   *
   * @return Robot position with Y-axis inverted for QML display
   */
  QPointF getRobotScreenPos() const;

  /**
   * @brief Get robot rotation in degrees for screen display
   *
   * @return Robot rotation in degrees, inverted for QML display
   */
  double getRobotScreenRotation() const;

  /**
   * @brief Activate map subscriptions
   */
  Q_INVOKABLE void activateSubscriptions();

  /**
   * @brief Destroy subscriptions
   */
  Q_INVOKABLE void destroySubscriptions();

signals:
  void mapAvailableChanged();
  void mapImageChanged();
  void mapMetadataChanged();
  void robotPoseChanged();

private:
  /**
   * @brief Constructor of MapManager class
   */
  MapManager();

  /**
   * @brief Destructor of MapManager class
   */
  ~MapManager() override;

  /**
   * @brief Callback for /map topic
   * @param msg OccupancyGrid message from SLAM
   */
  void callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

  /**
   * @brief Callback for /map_updates topic (incremental updates)
   * @param msg OccupancyGridUpdate message from SLAM
   */
  void callbackMapUpdates(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg);

  /**
   * @brief Timer callback to emit robot pose updates
   */
  void callbackRobotPoseSignalTimer();

  /**
   * @brief Build complete topic name based on namespace mode
   *
   * @param topic_suffix Topic suffix
   * @return Complete topic name with or without namespace prefix
   */
  std::string buildTopicName(const std::string& topic_suffix) const;

  /**
   * @brief Normalize robot namespace
   *
   * @param robot_namespace Raw namespace string
   * @return Normalized namespace
   */
  QString normalizeNamespace(const QString& robot_namespace) const;

  /**
   * @brief Recreate subscriptions with current namespace configuration
   */
  void recreateSubscriptions();

  rclcpp::Node* parent_node_;                                                           /**< Pointer to the parent ROS node */
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr sub_map_;               /**< Map topic subscriber */
  rclcpp::Subscription<map_msgs::msg::OccupancyGridUpdate>::SharedPtr sub_map_updates_; /**< Map updates topic subscriber */
  rclcpp::TimerBase::SharedPtr timer_robot_pose_;                                       /**< Timer to emit robot pose updates */

  MapData map_data_;                      /**< Map data handler */
  std::unique_ptr<RobotPose> robot_pose_; /**< Robot pose handler */

  QString selected_robot_namespace_; /**< Current robot namespace */
  bool use_namespace_discovery_;     /**< If true, prepend namespace to topics */
  bool is_initialized_;              /**< Initialization flag */
};

} // namespace manager
} // namespace map
} // namespace ROBOGait
