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
   * @brief Get a QoS profile for reliable communication without latching
   *
   * This QoS is intended to be used in communications that require retransmission
   * and when a new subscriber joins, he does not receive the last published
   * messages.
   */
  static rclcpp::QoS QOS_RELIABLE();

  /**
   * @brief Get a QoS profile for reliable communication with latching
   *
   * This QoS is intended to be used in communications that require retransmission
   * and when a new subscriber joins, they receive the last published messages.
   */
  static rclcpp::QoS QOS_RELIABLE_LATCH();

  /**
   * @brief Get a QoS profile for best effort communication
   *
   * This QoS is intended to be used in communications that do not require
   * retransmission and when a new subscriber joins, he does not receive the last
   * published messages.
   */
  static rclcpp::QoS QOS_BEST_EFFORT();

  /**
   * @brief Get a QoS profile for service clients
   *
   * This quality of service is meant to be used in clients of services communications.
   */
  static rmw_qos_profile_t QOS_CLIENTS();
};
} // namespace ros
} // namespace ROBOGait