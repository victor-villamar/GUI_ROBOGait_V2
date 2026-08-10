#pragma once

#include <memory>
#include <optional>
#include <string>

#include <rclcpp/node.hpp>

#include <nav_msgs/msg/path.hpp>

#include "Context/RobotContext.hpp"
#include "Map/Data/PathData.hpp"

namespace ROBOGait
{
namespace map
{
namespace subscribers
{
/**
 * @brief Subscribes to path topics and fills PathData
 */
class PathSubscriber
{
public:
  /**
   * @brief Constructor of PathSubscriber class
   */
  PathSubscriber();

  /**
   * @brief Initialize the subscriber
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Sets the path data
   *
   * @param path_data The path data
   */
  void setPathData(data::PathData* path_data);

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
   * @brief Callback for path messages
   *
   * @param msg The path message
   */
  void callbackPath(const nav_msgs::msg::Path::SharedPtr msg);

  rclcpp::Node* parent_node_;                                     /**< Parent node for subscriptions */
  rclcpp::Subscription<nav_msgs::msg::Path>::SharedPtr sub_path_; /**< Subscription to the path topic */

  data::PathData* path_data_;                              /**< Path data */
  std::optional<ROBOGait::context::RobotContext> context_; /**< Robot context */
  bool active_;                                            /**< Flag indicating if subscriber is active */
};

} // namespace subscribers
} // namespace map
} // namespace ROBOGait
