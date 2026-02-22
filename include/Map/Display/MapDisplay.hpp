#pragma once

#include <QGraphicsPixmapItem>
#include <QImage>
#include <QMutex>
#include <QString>

#include <memory>
#include <string>

#include <map_msgs/msg/occupancy_grid_update.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/subscription.hpp>

#include "Map/Display/BaseDisplay.hpp"
#include "Map/MapData.hpp"

namespace ROBOGait
{
namespace map
{
namespace display
{

/**
 * @brief Display for occupancy grid map visualization
 *
 * This display subscribes to:
 * - /map (nav_msgs/OccupancyGrid) - Full map
 * - /map_updates (map_msgs/OccupancyGridUpdate) - Incremental updates
 *
 * Map coordinates are in meters:
 * - X axis: forward (right)
 * - Y axis: left (up)
 * - Origin: map origin from metadata
 */
class MapDisplay : public BaseDisplay
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of MapDisplay class
   *
   * @param parent Qt parent object
   */
  explicit MapDisplay(QObject* parent = nullptr);

  /**
   * @brief Destructor of MapDisplay class
   */
  ~MapDisplay() override;

  /**
   * @brief Initialize display with ROS2 parent node
   *
   * @param parent_node Pointer to the parent ROS2 node for subscriptions
   */
  void initialize(rclcpp::Node* parent_node) override;

  /**
   * @brief Shutdown display
   */
  void shutdown() override;

  /**
   * @brief Update display state
   *
   * @param wall_dt Wall time delta (seconds)
   * @param ros_dt ROS time delta (seconds)
   */
  void update(double wall_dt, double ros_dt) override;

  /**
   * @brief Get the graphics item for the map
   *
   * @return Graphics item for the map
   */
  QGraphicsItem* getGraphicsItem() override { return map_item_.get(); }

  /**
   * @brief Check if map is available
   *
   * @return True if map has been received
   */
  bool isMapAvailable() const { return map_data_ != nullptr && map_data_->isAvailable(); }

  /**
   * @brief Get map data
   *
   * @return Shared pointer to MapData
   */
  std::shared_ptr<MapData> getMapData() const { return map_data_; }

  /**
   * @brief Set selected robot configuration
   *
   * @param robot_identifier Robot namespace (e.g., "/robot1") or node name
   * @param is_namespace True if identifier is a namespace, false if it's a node name
   */
  void setSelectedRobot(const QString& robot_identifier, bool is_namespace);

  /**
   * @brief Activate subscriptions based on selected robot configuration
   *
   */
  void activateSubscriptions();

signals:
  void mapUpdated();         // Signal emitted when map is received or updated
  void mapMetadataChanged(); // Signal emitted when map metadata changes

private:
  /**
   * @brief Callback for received /map topic
   *
   * @param msg Occupancy grid message
   */
  void callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

  /**
   * @brief Callback for received /map_updates topic
   *
   * @param msg Occupancy grid update message
   */
  void callbackMapUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg);

  /**
   * @brief Update the map graphics item from map data
   */
  void updateMapGraphics();

  /**
   * @brief Normalize robot namespace
   *
   * @param robot_namespace Raw namespace string
   *
   * @return Normalized namespace (e.g., "/robot1")
   */
  QString normalizeNamespace(const QString& robot_namespace) const;

  /**
   * @brief Build topic name based on namespace configuration
   *
   * @param topic_suffix Topic suffix (e.g., "/map")
   *
   * @return Full topic name
   */
  std::string buildTopicName(const std::string& topic_suffix) const;

  /**
   * @brief Recreate subscriptions with current robot configuration
   */
  void recreateSubscriptions();

  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr sub_map_;              /**< Map subscription */
  rclcpp::Subscription<map_msgs::msg::OccupancyGridUpdate>::SharedPtr sub_map_update_; /**< Map update subscription */

  std::shared_ptr<MapData> map_data_;             /**< Map data container */
  std::unique_ptr<QGraphicsPixmapItem> map_item_; /**< Graphics item for map rendering */

  QMutex data_mutex_; /**< Mutex for thread-safe data access */

  bool map_graphics_dirty_;          /**< Flag to indicate map graphics need update */
  QString selected_robot_namespace_; /**< Selected robot namespace or identifier */
  bool use_namespace_discovery_;     /**< True if using namespace discovery, false if direct topic */
};

} // namespace display
} // namespace map
} // namespace ROBOGait
