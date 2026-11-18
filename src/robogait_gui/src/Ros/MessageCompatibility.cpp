#include "Ros/MessageCompatibility.hpp"

namespace ROBOGait
{
namespace ros
{
namespace compatibility
{
namespace
{

template <typename ByteContainer> std::vector<std::uint8_t> toByteVectorImpl(const ByteContainer& data)
{
#if defined(ROBOGAIT_ROS_HUMBLE) || defined(ROBOGAIT_ROS_JAZZY)
  return data;
#elif defined(ROBOGAIT_ROS_LYRICAL)
  return data.to_vector();
#else
#error "Unsupported ROS 2 distribution"
#endif
}

} // namespace

std::vector<std::uint8_t> toByteVector(const sensor_msgs::msg::Image& image) { return toByteVectorImpl(image.data); }

std::vector<std::uint8_t> toByteVector(const command_executor_msgs::srv::GetMapData::Response& response) { return toByteVectorImpl(response.pgm_content); }

} // namespace compatibility
} // namespace ros
} // namespace ROBOGait