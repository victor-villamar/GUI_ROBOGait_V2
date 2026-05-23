#pragma once

#include "Define.hpp"

// clang-format off

// Topics
#define T_CMD_VEL                   "/cmd_vel"
#define T_ROBOT_STATUS              "/" COMMAND_EXECUTOR"/robot_status"
#define T_MAP                       "/map"
#define T_MAP_UPDATES               "/map_updates"
#define T_PLAN                      "/plan"
#define T_SCAN                      "/scan"
#define T_PARTICLE_CLOUD            "/particle_cloud"
#define T_POSE_INITIALIZE           "/initialpose"
#define T_USER_DETECTION            "/user_detection_filter"

// Services
#define S_CMD                                        COMMAND_EXECUTOR"/cmd"
#define S_GET_MAP_DATA                               COMMAND_EXECUTOR"/get_map_data"
#define S_REINITIALIZE_GLOBAL_LOCALIZATION           "/reinitialize_global_localization"

// Actions
#define A_NAVIGATE_TO_POSE                         "/navigate_to_pose"
#define A_NAVIGATE_THROUGH_POSES                   "/navigate_through_poses"
#define A_COMPUTE_PATH_TO_POSE                     "/compute_path_to_pose"
#define A_COMPUTE_PATH_THROUGH_POSES               "/compute_path_through_poses"

// TF Frames
#define TF_MAP_FRAME                "map"
#define TF_ROBOT_FRAME              "base_link"

// clang-format on
