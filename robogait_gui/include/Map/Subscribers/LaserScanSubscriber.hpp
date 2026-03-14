#pragma once

#include <memory>
#include <optional>
#include <string>

#include <rclcpp/node.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include "Context/RobotContext.hpp"
#include "Map/Data/LaserScanData.hpp"

namespace ROBOGait
{
namespace map
{
namespace subscribers
{
/**
 * @brief Class for subscribing to laser scan data and updating the LaserScanData instance
 */
class LaserScanSubscriber
{
public:
  /**
   * @brief Constructor for the LaserScanSubscriber class
   */
  LaserScanSubscriber();

  /**
   * @brief Destructor for the LaserScanSubscriber class
   */
  ~LaserScanSubscriber();

  /**
   * @brief Initialize the LaserScanSubscriber
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Set the laser scan data
   *
   * @param laser_scan_data The LaserScanData instance
   */
  void setLaserScanData(data::LaserScanData* laser_scan_data);

  /**
   * @brief Set the robot context
   *
   * @param context The RobotContext instance
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Start the laser scan subscriber
   */
  void start();

  /**
   * @brief Stop the laser scan subscriber
   */
  void stop();

  /**
   * @brief Check if the laser scan subscriber is active
   *
   * @return True if active, false otherwise
   */
  bool isActive() const;

private:
  /**
   * @brief Callback function for laser scan messages
   *
   * @param msg The laser scan message
   */
  void callbackScan(const sensor_msgs::msg::LaserScan::SharedPtr msg);

  /**
   * @brief Transform the laser scan points from sensor frame to map frame
   *
   * @param msg The laser scan message
   * @param scan_frame The frame_id of the laser scan
   *
   * @return LaserScanMetadata with transformed points, or empty metadata on failure
   */
  data::LaserScanData::LaserScanMetadata transformLaserScan(const sensor_msgs::msg::LaserScan::SharedPtr msg, const std::string& scan_frame);

  rclcpp::Node* parent_node_;                                             /**< Pointer to the parent ROS node */
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_scan_; /**< Subscription to the laser scan topic */
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;                            /**< TF buffer for transform lookups */
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;               /**< TF listener for transform lookups */

  data::LaserScanData* laser_scan_data_; /**< Laser scan data */
  std::string map_frame_;                /**< The map frame for transforming laser scan points */
  bool active_;                          /**< Flag indicating if the subscriber is active */
  bool warn_logged_;                     /**< Flag indicating if a warning has been logged */

  std::optional<ROBOGait::context::RobotContext> context_;
};

} // namespace subscribers
} // namespace map
} // namespace ROBOGait
