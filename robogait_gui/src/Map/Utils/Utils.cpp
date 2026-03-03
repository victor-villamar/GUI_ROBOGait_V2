#include <cmath>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

#include "Map/Utils/Utils.hpp"

namespace ROBOGait
{
namespace map
{
namespace utils
{
double getYaw(const geometry_msgs::msg::Quaternion& quaternion)
{
  tf2::Quaternion tf_quat(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

  double roll, pitch, yaw;
  tf2::Matrix3x3(tf_quat).getRPY(roll, pitch, yaw);

  return yaw;
}

double rad2deg(double radians) { return radians * (180.0 / M_PI); }

double deg2rad(double degrees) { return degrees * (M_PI / 180.0); }
} // namespace utils
} // namespace map
} // namespace ROBOGait
