#pragma once

//------ TIME DEFINITIONS ------
// clang-format off
#define TIME_TO_ROBOT_POSE_UPDATE     100 // ms (10 Hz)
#define TIME_TO_ROBOT_TIMEOUT         500 // ms (2 Hz)
#define TIME_TO_PUBLISH_CMD_VEL       100  // ms (20 Hz)


//------ NODES NAME DEFINITIONS ------
#define ROBOGAIT_GUI                  "robogait_gui"
#define COMMAND_EXECUTOR              "command_executor"

// clang-format on

// Max time to wait for service response
#define SERVICE_CALL_TIMEOUT std::chrono::seconds(2)