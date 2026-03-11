#pragma once

//------ TIME DEFINITIONS ------
// clang-format off
#define TIME_MAIN_LOOP     1000 // ms (1 Hz)


//----- NODE NAME -----
#define COMMAND_EXECUTOR  "command_executor"

//----- TOPICS -----
#define T_ROBOT_STATUS  COMMAND_EXECUTOR"/robot_status"
#define S_CMD           COMMAND_EXECUTOR"/cmd"

// clang-format on

//---------------- PUBLISHER AND SUBSCRIPTION QoS SETTINGS ----------------
/*This QoS is intended to be used in communications that do not require
 * retransmission and when a new subscriber joins, he does not receive the
 * last published messages.*/
#define QOS_BEST_EFFORT                                                                                                                                        \
  rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))                                                                                    \
      .reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT)                                                                                                     \
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)                                                                                                          \
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)                                                                                                               \
      .keep_last(5)
