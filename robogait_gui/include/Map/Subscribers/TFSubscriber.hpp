#pragma once

#include <memory>
#include <optional>
#include <string>

#include <rclcpp/node.hpp>
#include <rclcpp/timer.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include "Context/RobotContext.hpp"
#include "Map/Data/RobotPoseData.hpp"

namespace ROBOGait
{
namespace map
{
namespace subscribers
{

/**
 * @brief Class to subscribe to TF updates and provide them to the robot pose data
 */
class TFSubscriber
{
public:
  /**
   * @brief Constructor of TFSubscriber class
   */
  TFSubscriber();

  /**
   * @brief Destructor of TFSubscriber class
   */
  ~TFSubscriber();

  /**
   * @brief Initialize TF subscriber
   *
   * @param parent_node The parent ROS node
   * @param map_frame The map frame ID
   * @param robot_frame The robot frame ID
   */
  void initialize(rclcpp::Node* parent_node, const std::string& map_frame, const std::string& robot_frame);

  /**
   * @brief Set the robot pose data
   *
   * @param robot_pose_data The robot pose data
   */
  void setRobotPoseData(data::RobotPoseData* robot_pose_data);

  /**
   * @brief Set the robot context
   *
   * @param context The robot context
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Start the TF subscriber
   */
  void start();

  /**
   * @brief Stop the TF subscriber
   */
  void stop();

  /**
   * @brief Pause or resume pose updates without stopping the subscriber
   *
   * @param paused True to pause updates, false to resume
   */
  void setPaused(bool paused);

  /**
   * @brief Check if the TF subscriber is active
   *
   * @return true if active, false otherwise
   */
  bool isActive() const;

private:
  /**
   * @brief Timer callback to update the robot pose from TF
   */
  void updatePoseFromTF();

  rclcpp::Node* parent_node_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::TimerBase::SharedPtr tf_timer_;

  data::RobotPoseData* robot_pose_data_;
  std::string map_frame_;
  std::string robot_frame_;
  bool active_;
  bool warn_logged_;
  bool paused_;

  std::optional<ROBOGait::context::RobotContext> context_;
};
} // namespace subscribers
} // namespace map
} // namespace ROBOGait