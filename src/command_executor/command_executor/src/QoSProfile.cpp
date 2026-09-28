#include "QoSProfile.hpp"

using namespace ROBOGait::ros;

rclcpp::QoS QosProfiles::QOS_BEST_EFFORT()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

#ifdef ROBOGAIT_ROS_HUMBLE
rmw_qos_profile_t QosProfiles::QOS_SERVICES() { return rmw_qos_profile_services_default; }
#elif defined(ROBOGAIT_ROS_JAZZY) || defined(ROBOGAIT_ROS_LYRICAL)
rclcpp::QoS QosProfiles::QOS_SERVICES() { return rclcpp::ServicesQoS(); }
#else
#error "Unsupported ROS 2 distribution"
#endif