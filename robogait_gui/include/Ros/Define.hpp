#pragma once

#include <chrono>
#include <string>

namespace ROBOGait
{
namespace ros
{
namespace define
{

//------ TIME DEFINITIONS ------
extern const std::chrono::milliseconds TIME_TO_ROBOT_POSE_UPDATE_MS; // (10 Hz)
extern const std::chrono::milliseconds TIME_TO_ROBOT_TIMEOUT_MS;     // (2 Hz)
extern const std::chrono::milliseconds TIME_TO_PUBLISH_CMD_VEL_MS;   // (20 Hz)

//------ NODES NAME DEFINITIONS ------
extern const std::string ROBOGAIT_GUI;
extern const std::string COMMAND_EXECUTOR;

// Max time to wait for service response
extern const std::chrono::seconds SERVICE_CALL_TIMEOUT;

} // namespace define
} // namespace ros
} // namespace ROBOGait
