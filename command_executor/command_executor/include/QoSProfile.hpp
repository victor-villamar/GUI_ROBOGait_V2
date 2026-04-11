#pragma once

#include <rclcpp/qos.hpp>
#include <rmw/types.h>

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
};
} // namespace ros
} // namespace ROBOGait