#pragma once

#include <memory>

#include <rclcpp/node.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

#include "Context/RobotContext.hpp"
#include "Map/Data/LaserScanData.hpp"

namespace ROBOGait
{
namespace map
{
namespace data
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
  ~LaserScanSubscriber() = default;

  /**
   * @brief Initialize the LaserScanSubscriber
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Set the laser scan data
   *
   * @param scan_data The LaserScanData instance
   */
  void setScanData(const std::shared_ptr<LaserScanData>& scan_data);

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

  rclcpp::Node* parent_node_;                                             /**< Pointer to the parent ROS node */
  std::shared_ptr<LaserScanData> scan_data_;                              /**< Laser scan data */
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_scan_; /**< Subscription to the laser scan topic */

  ROBOGait::context::RobotContext context_; /**< The robot context */
  bool has_context_;                        /**< Flag indicating if the context is set */
  bool active_;                             /**< Flag indicating if the subscriber is active */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
