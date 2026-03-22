#pragma once

#include "Define.hpp"

// clang-format off

// Topics
#define T_CMD_VEL                   "/cmd_vel"
#define T_ROBOT_STATUS              "/robot_status"
#define T_MAP                       "/map"
#define T_MAP_UPDATES               "/map_updates"
#define T_SCAN                      "/scan"
#define T_POSE_INITIALIZE          "/initialpose"

// Services
#define S_CMD                       COMMAND_EXECUTOR"/cmd"
#define S_GET_MAP_DATA              COMMAND_EXECUTOR"/get_map_data"

// TF Frames
#define TF_MAP_FRAME                "map"
#define TF_ROBOT_FRAME              "base_link"

// clang-format on