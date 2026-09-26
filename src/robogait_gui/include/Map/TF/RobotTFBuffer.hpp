#pragma once

#include <memory>
#include <optional>
#include <string>

#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>

#include <tf2_msgs/msg/tf_message.hpp>
#include <tf2_ros/buffer.h>

#include "Context/RobotContext.hpp"

namespace ROBOGait
{
namespace map
{
namespace tf
{

/**
 * @brief Owns the TF buffer and subscriptions for the currently selected robot
 */
class RobotTFBuffer
{
public:
  RobotTFBuffer();
  ~RobotTFBuffer();

  /**
   * @brief Set the ROS node used to create TF subscriptions
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Set the selected robot context
   *
   * If the provider is active, its subscriptions and buffer are recreated for
   * the new context so transforms from different robots cannot be mixed.
   *
   * @param context The selected robot context
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Create a fresh buffer and subscribe to the selected robot TF topics
   */
  void start();

  /**
   * @brief Destroy the TF subscriptions and their associated buffer
   */
  void stop();

  /**
   * @brief Get the TF buffer populated by the selected robot
   *
   * @return The active buffer, or nullptr while stopped
   */
  std::shared_ptr<tf2_ros::Buffer> getBuffer() const;

  /**
   * @brief Check whether the provider is subscribed to TF topics
   *
   * @return true if active, false otherwise
   */
  bool isActive() const;

private:
  using TFMessage = tf2_msgs::msg::TFMessage;
  using TFSubscription = rclcpp::Subscription<TFMessage>;

  void callbackTF(const TFMessage::ConstSharedPtr msg);
  void callbackTFStatic(const TFMessage::ConstSharedPtr msg);
  static void insertTransforms(const TFMessage::ConstSharedPtr& msg, const std::shared_ptr<tf2_ros::Buffer>& buffer, const std::string& authority,
                               bool is_static);

  rclcpp::Node* parent_node_;
  std::optional<ROBOGait::context::RobotContext> context_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  TFSubscription::SharedPtr sub_tf_;
  TFSubscription::SharedPtr sub_tf_static_;
  std::string tf_topic_;
  std::string tf_static_topic_;
  bool active_;
};

} // namespace tf
} // namespace map
} // namespace ROBOGait