#include <rmw/qos_profiles.h>
#include <tf2_ros/qos.hpp>

#include "Ros/QoSProfiles.hpp"

namespace ROBOGait
{
namespace ros
{
namespace qos_profiles
{

rclcpp::QoS QOS_RELIABLE()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

rclcpp::QoS QOS_RELIABLE_LATCH()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

rclcpp::QoS QOS_BEST_EFFORT()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

rclcpp::QoS QOS_TF_DYNAMIC() { return tf2_ros::DynamicListenerQoS(); }

rclcpp::QoS QOS_TF_STATIC() { return tf2_ros::StaticListenerQoS(); }

#ifdef ROBOGAIT_ROS_HUMBLE
rmw_qos_profile_t QOS_CLIENTS()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(10)
      .get_rmw_qos_profile();
}
#elif defined(ROBOGAIT_ROS_JAZZY) || defined(ROBOGAIT_ROS_LYRICAL)
rclcpp::QoS QOS_CLIENTS()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(10);
}
#else
#error "Unsupported ROS 2 distribution"
#endif
} // namespace qos_profiles
} // namespace ros
} // namespace ROBOGait