#include "Define.hpp"

namespace ROBOGait::command::define
{

const std::int32_t TIME_MAIN_LOOP = 1000;

const std::string COMMAND_EXECUTOR = "command_executor";
const std::string ROVER_MINI = "rover_mini";

const std::string T_ROBOT_STATUS = COMMAND_EXECUTOR + "/robot_status";
const std::string T_BATTERY_STATUS = ROVER_MINI + "/battery_status";
const std::string S_CMD = COMMAND_EXECUTOR + "/cmd";
const std::string S_GET_MAP_DATA = COMMAND_EXECUTOR + "/get_map_data";

} // namespace ROBOGait::command::define
