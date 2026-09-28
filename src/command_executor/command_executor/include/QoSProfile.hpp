#pragma once

#include <rclcpp/qos.hpp>

#ifdef ROBOGAIT_ROS_HUMBLE
#include <rmw/types.h>
#endif

namespace ROBOGait
{
namespace ros
{
/**
 * @brief Class containing predefined QoS profiles for the application
 */
class QosProfiles
{
public:
  /**
   * @brief Get a QoS profile for best effort communication
   *
   * This QoS is intended to be used in communications that do not require
   * retransmission and when a new subscriber joins, he does not receive the last
   * published messages.
   */
  static rclcpp::QoS QOS_BEST_EFFORT();

  /**
   * @brief Get the default QoS profile for service servers
   *
   * This quality of service is intended for service server communications
   */
#ifdef ROBOGAIT_ROS_HUMBLE
  static rmw_qos_profile_t QOS_SERVICES();
#elif defined(ROBOGAIT_ROS_JAZZY) || defined(ROBOGAIT_ROS_LYRICAL)
  static rclcpp::QoS QOS_SERVICES();
#else
#error "Unsupported ROS 2 distribution"
#endif
};
} // namespace ros
} // namespace ROBOGait