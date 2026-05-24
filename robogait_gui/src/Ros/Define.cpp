#include "Ros/Define.hpp"

namespace ROBOGait
{
namespace ros
{
namespace define
{
// clang-format off
const std::chrono::milliseconds TIME_TO_ROBOT_POSE_UPDATE_MS(100);
const std::chrono::milliseconds TIME_TO_ROBOT_TIMEOUT_MS(500);
const std::chrono::milliseconds TIME_TO_PUBLISH_CMD_VEL_MS(100);

const std::string               ROBOGAIT_GUI = "robogait_gui";
const std::string               COMMAND_EXECUTOR = "command_executor";

const std::chrono::seconds      SERVICE_CALL_TIMEOUT(2);
// clang-format on
} // namespace define
} // namespace ros
} // namespace ROBOGait
