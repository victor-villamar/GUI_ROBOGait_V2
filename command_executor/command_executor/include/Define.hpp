#pragma once

//------ TIME DEFINITIONS ------
// clang-format off
#define TIME_MAIN_LOOP     1000 // ms (1 Hz)


//----- NODE NAME -----
#define COMMAND_EXECUTOR  "command_executor"

//----- TOPICS -----
#define T_ROBOT_STATUS  COMMAND_EXECUTOR"/robot_status"
#define S_CMD           COMMAND_EXECUTOR"/cmd"
#define S_GET_MAP_DATA  COMMAND_EXECUTOR"/get_map_data"

// clang-format on