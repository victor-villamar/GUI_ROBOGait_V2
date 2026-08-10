#pragma once

#include <string>

namespace ROBOGait
{
namespace ros
{
namespace topics
{
// Topics
extern const std::string T_CMD_VEL;
extern const std::string T_ROBOT_STATUS;
extern const std::string T_MAP;
extern const std::string T_MAP_UPDATES;
extern const std::string T_PLAN;
extern const std::string T_SCAN;
extern const std::string T_PARTICLE_CLOUD;
extern const std::string T_POSE_INITIALIZE;
extern const std::string T_CAMERA_DETECTION;

// Services
extern const std::string S_CMD;
extern const std::string S_GET_MAP_DATA;
extern const std::string S_REINITIALIZE_GLOBAL_LOCALIZATION;

// Actions
extern const std::string A_NAVIGATE_TO_POSE;
extern const std::string A_COMPUTE_PATH_TO_POSE;
extern const std::string A_FOLLOW_PATH;

// TF Frames
extern const std::string TF_MAP_FRAME;
extern const std::string TF_ROBOT_FRAME;
} // namespace topics
} // namespace ros
} // namespace ROBOGait
