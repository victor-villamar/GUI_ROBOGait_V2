#pragma once

//------ TIME DEFINITIONS ------
// clang-format off
#define TIME_TO_ROBOT_POSE_UPDATE     100 // ms (10 Hz)
#define TIME_TO_ROBOT_TIMEOUT         500 // ms (2 Hz)


//------ NODES NAME DEFINITIONS ------
#define ROBOGAIT_GUI                  "robogait_gui"
#define COMMAND_EXECUTOR               "command_executor"

// clang-format on

// Max time to wait for service response
#define SERVICE_CALL_TIMEOUT std::chrono::seconds(2)

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

// This quality of service is meant to be used in clients of services communications
#define QOS_CLIENTS                                                                                                                                            \
  rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_default))                                                                                    \
      .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)                                                                                                        \
      .durability(RMW_QOS_POLICY_DURABILITY_VOLATILE)                                                                                                          \
      .history(RMW_QOS_POLICY_HISTORY_KEEP_LAST)                                                                                                               \
      .keep_last(10)                                                                                                                                           \
      .get_rmw_qos_profile()
