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