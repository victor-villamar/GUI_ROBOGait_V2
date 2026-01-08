#pragma once

#include <string>
#include <vector>

#include "enum.hpp"

namespace ROBOGait
{
namespace common
{
/**
 * @brief Struct representing the configuration of a process.
 */
struct ProcessConfig
{
  std::string name;
  std::string executable;
  std::vector<std::string> arguments;
  WindowState window_state;
};

/**
 * @brief Struct representing the configuration of a database.
 */
struct DatabaseConfig
{
  std::string name;
  std::string path;
  std::string script;
};
} // namespace common
} // namespace ROBOGait