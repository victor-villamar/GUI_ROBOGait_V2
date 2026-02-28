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
 * @brief Subscribes to map topics and fills MapData.
 */
class MapSubscriber
{
public:
  MapSubscriber();

  void initialize(rclcpp::Node* parent_node);
  void setMapData(const std::shared_ptr<MapData>& map_data);
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  void start();
  void stop();
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

  std::shared_ptr<MapData> map_data_;       /**< Map data */
  ROBOGait::context::RobotContext context_; /**< Robot context */
  bool has_context_;                        /**< Flag indicating if context is set */
  bool active_;                             /**< Flag indicating if subscriber is active */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
