#pragma once

#include <geometry_msgs/msg/quaternion.hpp>

namespace ROBOGait
{
namespace map
{
namespace utils
{

/**
 * @brief Extract yaw angle from ROS2 Quaternion message
 *
 * Converts quaternion (x, y, z, w) to yaw angle (rotation around Z-axis)
 * using tf2 library. This is the standard method for 2D robot orientation.
 *
 * @param quaternion ROS2 Quaternion message
 * @return Yaw angle in radians (-π to π)
 */
double getYaw(const geometry_msgs::msg::Quaternion& quaternion);

/**
 * @brief Convert radians to degrees
 *
 * @param radians Angle in radians
 * @return Angle in degrees
 */
double rad2deg(double radians);

/**
 * @brief Convert degrees to radians
 *
 * @param degrees Angle in degrees
 * @return Angle in radians
 */
double deg2rad(double degrees);

} // namespace utils
} // namespace map
} // namespace ROBOGait
