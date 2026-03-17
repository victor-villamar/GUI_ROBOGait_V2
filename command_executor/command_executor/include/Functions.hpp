#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ROBOGait
{
namespace functions
{
/**
 * @brief Get the YAML info for a map
 *
 * @param map_directory The directory where the map files are located
 * @param map_name The name of the map (without extension)
 *
 * @return The YAML info as a string, or std::nullopt if an error occurred
 */
std::optional<std::string> getMapYamlInfo(const std::string& map_directory, const std::string& map_name);

/**
 * @brief Get the PGM info for a map
 *
 * @param map_directory The directory where the map files are located
 * @param map_name The name of the map (without extension)
 *
 * @return The PGM info as a vector of uint8_t values, or std::nullopt if an error occurred
 */
std::optional<std::vector<uint8_t>> getMapPgmInfo(const std::string& map_directory, const std::string& map_name);

/**
 * @brief Expand a file path, resolving environment variables and user home directory
 *
 * @param path The file path to expand
 *
 * @return The expanded file path
 */
std::string expandPath(const std::string& path);
} // namespace functions
} // namespace ROBOGait