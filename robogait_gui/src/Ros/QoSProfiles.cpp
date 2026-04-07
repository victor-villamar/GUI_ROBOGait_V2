#include "Ros/QoSProfiles.hpp"

using namespace ROBOGait::ros;

rclcpp::QoS QosProfiles::QOS_RELIABLE()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

rclcpp::QoS QosProfiles::QOS_RELIABLE_LATCH()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

rclcpp::QoS QosProfiles::QOS_BEST_EFFORT()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(5);
}

rmw_qos_profile_t QosProfiles::QOS_CLIENTS()
{
  return rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)
      .keep_last(10)
      .get_rmw_qos_profile();
}