#pragma once

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <filesystem>
#include <string>

#include "logs/Logs.hpp"

using namespace ROBOGait::common::logs;

namespace ROBOGait
{
namespace common
{
namespace functions
{
/**
 * @brief Get the full file path given a package name and relative path
 * @param package_name The name of the ROS2 package
 * @param relative_path The relative path within the package
 * @return The full file path
 */
inline std::string getFilePath(const std::string& package_name, const std::string& relative_path)
{
  const std::string share_path = ament_index_cpp::get_package_share_directory(package_name);
  const std::string full_path = share_path + "/" + relative_path;

  if (!std::filesystem::exists(full_path))
  {
    Logs::error("[YamlLoader::getFilePath] File does not exist: %s", full_path.c_str());
    return "";
  }

  return full_path;
}
} // namespace functions
} // namespace common
} // namespace ROBOGait