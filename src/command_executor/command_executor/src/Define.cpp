#include "Define.hpp"

namespace ROBOGait
{
namespace command
{

namespace define
{
// clang-format off
//------ TIME DEFINITIONS ------
const std::chrono::milliseconds TIME_MAIN_LOOP_MS(1000);

//----- NODE NAME -----
const std::string COMMAND_EXECUTOR = "command_executor";
const std::string ROVER_MINI =       "rover_mini";

//----- TOPICS -----
const std::string T_ROBOT_STATUS =   COMMAND_EXECUTOR + "/robot_status";
const std::string T_BATTERY_STATUS = ROVER_MINI + "/battery_status";

//----- SERVICES -----
const std::string S_CMD =            COMMAND_EXECUTOR + "/cmd";
const std::string S_GET_MAP_DATA =   COMMAND_EXECUTOR + "/get_map_data";
// clang-format on
} // namespace define
} // namespace command
} // namespace ROBOGait
