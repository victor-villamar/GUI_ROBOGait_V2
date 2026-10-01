#pragma once

#include <cstdint>
#include <vector>

#include <command_executor_msgs/srv/get_map_data.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace ROBOGait
{
namespace ros
{
namespace compatibility
{

/**
 * @brief Convert the byte data from a ROS image to an owning standard vector.
 */
std::vector<std::uint8_t> toByteVector(const sensor_msgs::msg::Image& image);

/**
 * @brief Convert the PGM data from a map service response to an owning standard vector.
 */
std::vector<std::uint8_t> toByteVector(const command_executor_msgs::srv::GetMapData::Response& response);

} // namespace compatibility
} // namespace ros
} // namespace ROBOGait