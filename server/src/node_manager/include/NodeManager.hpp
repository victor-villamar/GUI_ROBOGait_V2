#pragma once

#include <interface_srv/srv/manage_process.hpp>
#include <memory>
#include <rclcpp/node.hpp>

#include "StateMachine.hpp"

namespace ROBOGait
{
namespace manager
{
/**
 * @brief Class for managing ROS2 nodes and related processes.
 */
class NodeManager
{
public:
  /**
   * @brief Construct a new Node Manager object
   *
   * @param parent_node Pointer to the parent ROS2 node
   */
  NodeManager(rclcpp::Node* parent_node);

  /**
   * @brief Destroy the Node Manager object
   */
  ~NodeManager();

  /**
   * @brief Deleted copy constructor to prevent copying
   */
  NodeManager(const NodeManager&) = delete;

  /**
   * @brief Deleted assignment operator to prevent copying
   */
  NodeManager& operator=(const NodeManager&) = delete;

  /**
   * @brief Initialize the Node Manager
   *
   * @return true if initialization was successful, false otherwise
   */
  bool init();

private:
  void handleProcessCommandRequest(const std::shared_ptr<interface_srv::srv::ManageProcess::Request>& request,
                                   std::shared_ptr<interface_srv::srv::ManageProcess::Response> response);

  rclcpp::Node* parent_node_;                                                        /**< Pointer to the parent ROS2 node */
  rclcpp::Service<interface_srv::srv::ManageProcess>::SharedPtr ser_manage_process_; /**< Service for managing process commands */
  std::unique_ptr<ROBOGait::state_machine::StateMachine> state_machine_;             /**< State machine instance */
};
} // namespace manager
} // namespace ROBOGait
