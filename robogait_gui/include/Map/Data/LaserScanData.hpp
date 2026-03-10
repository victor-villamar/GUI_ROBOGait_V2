#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include <QMutex>
#include <QPointF>

#include <rclcpp/node.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
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
 * @brief Class for handling laser scan data and TF updates related to the laser scan
 */
class LaserScanData
{
public:
  /**
   * @brief Constructor of LaserScanData class
   */
  LaserScanData();

  /**
   * @brief Destructor of LaserScanData class
   */
  ~LaserScanData();

  /**
   * @brief Initialize the LaserScan data
   *
   * @param parent_node The parent ROS node to use for TF listener and other ROS interactions
   */
  void initialize(rclcpp::Node* parent_node);

  /**
   * @brief Start the TF listener for laser scan data
   */
  void startTFListener();

  /**
   * @brief Stop the TF listener for laser scan data
   */
  void stopTFListener();

  /**
   * @brief Update the internal state from a LaserScan message
   *
   * @param msg The LaserScan message to update from
   */
  void updateFromLaserScan(const sensor_msgs::msg::LaserScan::SharedPtr msg);

  /**
   * @brief Get the points from the laser scan data
   *
   * @param points Vector to fill with the laser scan points
   *
   * @return true if points are available, false otherwise
   */
  bool getPoints(std::vector<QPointF>& points) const;

  /**
   * @brief Check if the laser scan data is available
   *
   * @return true if points are available, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Get the timestamp of the last update
   *
   * @return The timestamp of the last update
   */
  uint64_t getUpdateStamp() const;

  /**
   * @brief Reset the laser scan data
   */
  void reset();

  /**
   * @brief Set the robot context
   *
   * @param context The robot context to set
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Check if the robot context is set
   *
   * @return true if the robot context is set, false otherwise
   */
  bool hasRobotContext() const;

  /**
   * @brief Get the topic name for the laser scan data
   *
   * @return The topic name for the laser scan data
   */
  std::string getScanTopic() const;

private:
  rclcpp::Node* parent_node_;                               /**< The parent ROS node */
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;              /**< The TF2 buffer for storing transforms */
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_; /**< The TF2 transform listener */

  std::vector<QPointF> points_; /**< The points from the laser scan data */
  bool is_available_;           /**< Flag indicating if the data is available */
  bool warn_logged_;            /**< Flag indicating if a warning has been logged */
  uint64_t update_stamp_;       /**< The update timestamp */
  bool enabled_;                /**< Flag indicating if the listener is enabled */
  std::string map_frame_;       /**< The map frame ID */

  mutable QMutex data_mutex_; /**< Mutex for protecting shared data */

  ROBOGait::context::RobotContext context_; /**< The robot context */
  bool has_context_;                        /**< Flag indicating if the robot context is set */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
