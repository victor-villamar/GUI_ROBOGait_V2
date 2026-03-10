#pragma once

#include <rclcpp/callback_group.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/timer.hpp>

#include <command_executor_msgs/msg/robot_status.hpp>

#include <chrono>
#include <memory>

#include "Ros/Define.hpp"

#define T_ROBOT_STATUS "/robot_status"

/**
 * @brief Test node that publishes robot status information
 *
 * This node publishes robot name at 1Hz
 */
class TestNode
{
public:
  /**
   * @brief Constructor of TestNode class
   *
   * @param node Shared pointer to the parent ROS2 node
   */
  TestNode(std::shared_ptr<rclcpp::Node> node, const std::string& robot_namespace);

  /**
   * @brief Destructor of TestNode class
   */
  ~TestNode();

private:
  /**
   * @brief Timer callback that publishes robot name
   *
   * This function is called at 1Hz to publish robot name.
   */
  void publishLoop();

  std::shared_ptr<rclcpp::Node> parent_node_; /**< Parent ROS2 node */

  rclcpp::Publisher<command_executor_msgs::msg::RobotStatus>::SharedPtr pub_robot_status_; /**< Publisher for robot status */

  rclcpp::TimerBase::SharedPtr loop_timer_; /**< Timer for periodic publishing */

  rclcpp::CallbackGroup::SharedPtr cb_group_; /**< Callback group for timer */

  std::string robot_name_;      /**< Robot name */
  std::string robot_namespace_; /**< Namespace for the node */
};
