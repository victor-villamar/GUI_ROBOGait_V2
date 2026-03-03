#pragma once

//------ TIME DEFINITIONS ------
// clang-format off
#define TIME_MAIN_LOOP     1000 // ms (1 Hz)


//----- NODE NAME -----
#define CMD_EXECUTOR  "command_executor"

//----- TOPICS -----
#define T_ROBOT_STATUS  CMD_EXECUTOR"/robot_status"
#define S_CMD           CMD_EXECUTOR"/cmd"

// clang-format on

//---------------- PUBLISHER AND SUBSCRIPTION QoS SETTINGS ----------------
/*This QoS is intended to be used in communications that require
 * retransmission and when a new subscriber joins, he does not receive the
 * last published messages.*/
#define QOS_RELIABLE                                                                                                                                           \
  rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))                                                                                    \
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)                                                                                                        \
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)                                                                                                          \
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)                                                                                                               \
      .keep_last(5)

/*This QoS is intended to be used in communications that require
 * retransmission and when a new subscriber joins, they receive the last
 * published messages.*/
#define QOS_RELIABLE_LATCH                                                                                                                                     \
  rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))                                                                                    \
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)                                                                                                        \
      .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)                                                                                                   \
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)                                                                                                               \
      .keep_last(5)

/*This QoS is intended to be used in communications that do not require
 * retransmission and when a new subscriber joins, he does not receive the
 * last published messages.*/
#define QOS_BEST_EFFORT                                                                                                                                        \
  rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))                                                                                    \
      .reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT)                                                                                                     \
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)                                                                                                          \
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)                                                                                                               \
      .keep_last(5)
