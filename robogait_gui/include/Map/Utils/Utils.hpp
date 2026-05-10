#pragma once

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <QImage>
#include <QString>
#include <QVariantList>

#include <tf2/LinearMath/Quaternion.h>

#include <geometry_msgs/msg/quaternion.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "Map/Data/MapData.hpp"

namespace ROBOGait
{
namespace map
{
namespace utils
{

static constexpr double RAD2DEG = 180.0 / M_PI;    /**< Radians to degrees conversion factor */
static constexpr double DEG2RAD = M_PI / 180.0;    /**< Degrees to radians conversion factor */
static constexpr int8_t UNKNOWN_OCCUPANCY = -1;    /**< Unknown occupancy value */
static constexpr int8_t FREE_SPACE_THRESHOLD = 50; /**< Free space threshold */
static constexpr int IMAGE_QUALITY = 100;          /**< Image quality */

/**
 * @brief Waypoint input structure for navigation goals
 *
 * @param x X coordinate in meters
 * @param y Y coordinate in meters
 * @param theta Optional orientation in radians (yaw)
 */
struct WaypointInput
{
  double x = 0.0;
  double y = 0.0;
  std::optional<double> theta;
};

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
 * @brief Create a ROS2 Quaternion message from a yaw angle
 *
 * @param yaw Yaw angle in radians
 *
 * @return The quaternion message representing the given yaw angle (rotation around Z-axis)
 */
geometry_msgs::msg::Quaternion createQuaternionFromYaw(double yaw);

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
 * @brief Convert MapData to QImage
 *
 * Colormap:
 * - Unknown (-1): dark blue-gray
 * - Free space (< 50): light blue
 * - Occupied (>= 50): white
 *
 * @param map_data MapData object
 *
 * @return QImage representation of the map data
 */
QImage toQImage(const data::MapData& map_data);

/**
 * @brief Generate PNG preview for the given map data
 *
 * @param map_data Shared pointer to the map data
 * @param map_name Name of the map
 *
 * @return true if the preview was generated successfully, false otherwise
 */
bool generateMapPreview(const data::MapData& map_data, const QString& map_name);

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

/**
 * @brief Parse waypoint list coming from Qt into strongly typed inputs
 *
 * @param points List with entries containing x, y and optional theta
 * @return Parsed and validated waypoint list
 */
std::vector<WaypointInput> parseWaypointInputs(const QVariantList& points);

/**
 * @brief Build waypoint orientation quaternion
 *
 * Uses theta if available, otherwise returns identity orientation.
 *
 * @param waypoint Parsed waypoint
 * @return Waypoint orientation quaternion
 */
geometry_msgs::msg::Quaternion buildWaypointOrientation(const WaypointInput& waypoint);

} // namespace utils
} // namespace map
} // namespace ROBOGait
