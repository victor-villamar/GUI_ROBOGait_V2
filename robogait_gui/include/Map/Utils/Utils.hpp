#pragma once

#include <memory>
#include <string>

#include <QString>

#include <geometry_msgs/msg/quaternion.hpp>

#include "Map/Data/MapData.hpp"

namespace ROBOGait
{
namespace map
{
namespace utils
{

/**
 * @brief Extract yaw angle from ROS2 Quaternion message
 *
 * Converts quaternion (x, y, z, w) to yaw angle (rotation around Z-axis)
 * using tf2 library. This is the standard method for 2D robot orientation.
 *
 * @param quaternion ROS2 Quaternion message
 * @return Yaw angle in radians (-π to π)
 */
double getYaw(const geometry_msgs::msg::Quaternion& quaternion);

/**
 * @brief Convert radians to degrees
 *
 * @param radians Angle in radians
 * @return Angle in degrees
 */
double rad2deg(double radians);

/**
 * @brief Convert degrees to radians
 *
 * @param degrees Angle in degrees
 * @return Angle in radians
 */
double deg2rad(double degrees);

/**
 * @brief Generate PNG preview for the given map data
 *
 * @param map_data Shared pointer to the map data
 * @param map_name Name of the map
 *
 * @return true if the preview was generated successfully, false otherwise
 */
bool generateMapPreview(const std::shared_ptr<data::MapData>& map_data, const QString& map_name);

/**
 * @brief Delete the generated map preview
 *
 * @param map_name Name of the map
 * @return true if the preview was deleted successfully, false otherwise
 */
bool deleteMapPreview(const QString& map_name);

/**
 * @brief Get the file path of the generated map preview
 *
 * @param map_name Name of the map
 *
 * @return File path of the generated PNG preview
 */
QString getMapPreviewPath(const QString& map_name);

/**
 * @brief Sanitize map name for filesystem usage
 *
 * Replaces spaces and unsafe characters with underscores.
 *
 * @param map_name Original map name
 *
 * @return Sanitized map name safe for file paths
 */
std::string sanitizeMapName(const std::string& map_name);

} // namespace utils
} // namespace map
} // namespace ROBOGait
