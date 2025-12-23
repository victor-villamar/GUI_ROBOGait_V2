#pragma once

#define EN_CASA 0
#define ROBOT 1
#define NAV 0

// clang-format off
/** Node names */
#define NODE_MANAGER_NODE_NAME                     "node_manager"
// clang-format on

#define SERVICE_CALL_TIMEOUT std::chrono::seconds(2)

/*---------------- QoS SETTINGS -----------------*/
/*This QoS is meant to be used in clients of services communications*/
#define QOS_CLIENTS                                                                                                                                            \
  rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_services_default))                                                                           \
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)                                                                                                        \
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)                                                                                                          \
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)                                                                                                               \
      .keep_last(queue_size)
