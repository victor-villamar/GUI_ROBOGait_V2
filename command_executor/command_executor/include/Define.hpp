#pragma once

#include <cstdint>
#include <string>

namespace ROBOGait
{
namespace command
{
namespace define
{
// clang-format off
//------ TIME DEFINITIONS ------
extern const std::int32_t TIME_MAIN_LOOP; // ms (1 Hz)

//----- NODE NAME -----
extern const std::string  COMMAND_EXECUTOR;
extern const std::string  ROVER_MINI;

//----- TOPICS -----
extern const std::string  T_ROBOT_STATUS;
extern const std::string  T_BATTERY_STATUS;
extern const std::string  S_CMD;
extern const std::string  S_GET_MAP_DATA;
// clang-format on
} // namespace define
} // namespace command
} // namespace ROBOGait