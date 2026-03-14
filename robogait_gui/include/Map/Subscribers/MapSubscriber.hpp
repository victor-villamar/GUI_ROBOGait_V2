#pragma once

#include <memory>

#include <map_msgs/msg/occupancy_grid_update.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>

#include "Context/RobotContext.hpp"
#include "Map/Data/MapData.hpp"

namespace ROBOGait
{
namespace map
{
namespace data
{

/**
 * @brief Subscribes to map topics and fills and populates MapData
 */
class MapSubscriber
{
public:
  /**
   * @brief Constructor of MapSubscriber class
   */
  MapSubscriber();

  /**
   * @brief Initializes the subscriber
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Sets the map data
   *
   * @param map_data The map data
   */
  void setMapData(MapData* map_data);

  /**
   * @brief Sets the robot context
   *
   * @param context The robot context
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Starts the subscriber
   */
  void start();

  /**
   * @brief Stops the subscriber
   */
  void stop();

  /**
   * @brief Checks if the subscriber is active
   *
   * @return True if active, false otherwise
   */
  bool isActive() const;

private:
  /**
   * @brief Callback for map messages
   *
   * @param msg The map message
   */
  void callbackMap(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

  /**
   * @brief Callback for map update messages
   *
   * @param msg The map update message
   */
  void callbackMapUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg);

  rclcpp::Node* parent_node_;                                                          /**< Parent node for subscriptions */
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr sub_map_;              /**< Subscription to the map topic */
  rclcpp::Subscription<map_msgs::msg::OccupancyGridUpdate>::SharedPtr sub_map_update_; /**< Subscription to the map update topic */

  MapData* map_data_;                       /**< Map data */
  ROBOGait::context::RobotContext context_; /**< Robot context */
  bool has_context_;                        /**< Flag indicating if context is set */
  bool active_;                             /**< Flag indicating if subscriber is active */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
