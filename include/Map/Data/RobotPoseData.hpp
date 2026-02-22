#pragma once

#include <memory>
#include <rclcpp/node.hpp>
#include <rclcpp/timer.hpp>
#include <string>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include "Context/RobotContext.hpp"

namespace ROBOGait
{
namespace map
{
namespace data
{

/**
 * @brief Class to store and manage robot pose (position and orientation)
 */
class RobotPoseData
{
public:
  /**
   * @brief Constructor of RobotPoseData class
   *
   * @param parent_node Pointer to parent ROS2 node
   * @param map_frame TF frame for the map (default: "map")
   * @param robot_frame TF frame for the robot base (default: "base_link")
   * @param update_rate Update frequency in Hz (default: 10.0)
   */
  RobotPoseData(rclcpp::Node* parent_node, const std::string& map_frame = "map", const std::string& robot_frame = "base_link", double update_rate = 10.0);

  /**
   * @brief Destructor of RobotPoseData class
   */
  ~RobotPoseData();

  /**
   * @brief Get robot X position in map frame
   *
   * @return X coordinate in meters
   */
  double getX() const;

  /**
   * @brief Get robot Y position in map frame
   *
   * @return Y coordinate in meters
   */
  double getY() const;

  /**
   * @brief Get robot orientation (yaw angle) in map frame
   *
   * @return Theta angle in radians (-π to π)
   */
  double getTheta() const;

  /**
   * @brief Check if robot pose has been initialized
   *
   * @return true if pose data is available, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Reset pose to default values (origin)
   */
  void reset();

  /**
   * @brief Provide robot context for frame resolution
   *
   * @param context Robot context with namespace info
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Check if a context has been provided
   */
  bool hasRobotContext() const { return has_context_; }

private:
  /**
   * @brief Timer callback to update pose from TF
   */
  void updatePoseFromTF();

  rclcpp::Node* parent_node_;                               /**< Parent ROS2 node pointer */
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;              /**< TF2 buffer for transform lookups */
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_; /**< TF2 listener */
  rclcpp::TimerBase::SharedPtr tf_timer_;                   /**< Timer for periodic TF updates */

  std::string map_frame_;   /**< Map frame name */
  std::string robot_frame_; /**< Robot base frame name */

  double x_;          /**< Robot X position in meters (map frame) */
  double y_;          /**< Robot Y position in meters (map frame) */
  double theta_;      /**< Robot orientation in radians (map frame) */
  bool is_available_; /**< Flag indicating if pose has been received */
  bool warn_logged_;  /**< Flag to avoid spamming warnings */

  ROBOGait::context::RobotContext context_; /**< Robot context for frame resolution */
  bool has_context_;                        /**< Flag indicating if context is set */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
