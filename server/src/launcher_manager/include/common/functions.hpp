#pragma once

#include <string>

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
std::string getFilePath(const std::string& package_name, const std::string& relative_path);

/**
 * @brief Replace spaces in a string with a special character
 * @param name The input string
 * @return The modified string with spaces replaced
 */
std::string replaceSpaces(const std::string& name);

/**
 * @brief Delete map files (YAML and PGM) given the map path and name
 * @param map_path The path to the maps directory
 * @param map_name The name of the map
 *
 * @return true if the map files were deleted successfully, false otherwise
 */
bool deleteMapFile(const std::string& map_path, const std::string& map_name);
} // namespace functions
} // namespace common
} // namespace ROBOGait