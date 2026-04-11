#pragma once

#include <memory>
#include <optional>
#include <string>

#include <rclcpp/node.hpp>

#include <nav2_msgs/msg/particle_cloud.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include "Context/RobotContext.hpp"
#include "Map/Data/ParticleCloudData.hpp"

namespace ROBOGait
{
namespace map
{
namespace subscribers
{
/**
 * @brief Class for subscribing to particle cloud data and updating the ParticleCloudData instance
 */
class ParticleCloudSubscriber
{
public:
  /**
   * @brief Constructor for the ParticleCloudSubscriber class
   */
  ParticleCloudSubscriber();

  /**
   * @brief Destructor for the ParticleCloudSubscriber class
   */
  ~ParticleCloudSubscriber();

  /**
   * @brief Initialize the ParticleCloudSubscriber
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Set the particle cloud data
   *
   * @param particle_cloud_data The ParticleCloudData instance
   */
  void setParticleCloudData(data::ParticleCloudData* particle_cloud_data);

  /**
   * @brief Set the robot context
   *
   * @param context The RobotContext instance
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Start the particle cloud subscriber
   */
  void start();

  /**
   * @brief Stop the particle cloud subscriber
   */
  void stop();

  /**
   * @brief Check if the particle cloud subscriber is active
   *
   * @return True if active, false otherwise
   */
  bool isActive() const;

private:
  /**
   * @brief Callback function for particle cloud messages
   *
   * @param msg The particle cloud message
   */
  void callbackParticleCloud(const nav2_msgs::msg::ParticleCloud::SharedPtr msg);

  /**
   * @brief Transform the particle cloud poses to map frame if needed
   *
   * @param msg The particle cloud message
   * @param cloud_frame The frame_id of the particle cloud
   *
   * @return ParticleCloudMetadata with transformed particles, or empty metadata on failure
   */
  data::ParticleCloudData::ParticleCloudMetadata transformParticleCloud(const nav2_msgs::msg::ParticleCloud::SharedPtr msg, const std::string& cloud_frame);

  rclcpp::Node* parent_node_;                                                /**< Pointer to the parent ROS node */
  rclcpp::Subscription<nav2_msgs::msg::ParticleCloud>::SharedPtr sub_cloud_; /**< Subscription to the particle cloud topic */
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;                               /**< TF buffer for transform lookups */
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;                  /**< TF listener for transform lookups */

  data::ParticleCloudData* particle_cloud_data_; /**< Particle cloud data */
  std::string map_frame_;                        /**< The map frame for transforming particles */
  bool active_;                                  /**< Flag indicating if the subscriber is active */
  bool warn_logged_;                             /**< Flag indicating if a warning has been logged */

  std::optional<ROBOGait::context::RobotContext> context_;
};

} // namespace subscribers
} // namespace map
} // namespace ROBOGait
