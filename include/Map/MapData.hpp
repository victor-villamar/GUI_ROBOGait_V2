#pragma once

#include <QImage>
#include <map_msgs/msg/occupancy_grid_update.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <stdint.h>
#include <vector>

namespace ROBOGait
{
namespace map
{

/**
 * @brief Map metadata
 *
 * @param resolution Map resolution in meters per pixel
 * @param width Map width in cells
 * @param height Map height in cells
 * @param origin_x Map origin X coordinate in meters (map frame)
 * @param origin_y Map origin Y coordinate in meters (map frame)
 * @param origin_theta Map origin rotation in radians
 */
struct MapMetadata
{
  double resolution;   /**< Map resolution in meters per pixel */
  uint32_t width;      /**< Map width in cells */
  uint32_t height;     /**< Map height in cells */
  double origin_x;     /**< Map origin X coordinate in meters (map frame) */
  double origin_y;     /**< Map origin Y coordinate in meters (map frame) */
  double origin_theta; /**< Map origin rotation in radians */

  MapMetadata() : resolution(0.05), width(0), height(0), origin_x(0.0), origin_y(0.0), origin_theta(0.0) {}
};

/**
 * @brief Class to store and process occupancy grid map data
 *
 * Occupancy values:
 * - -1 = unknown (rendered as gray)
 * - 0 = free space (rendered as white)
 * - 100 = occupied (rendered as black)
 * - 1-99 = probability of occupation (rendered as gradient)
 */
class MapData
{
public:
  /**
   * @brief Constructor of MapData class
   */
  MapData();

  /**
   * @brief Destructor of MapData class
   */
  ~MapData() = default;

  /**
   * @brief Update map data from ROS2 OccupancyGrid message
   *
   * @param msg ROS2 OccupancyGrid message from /map topic
   */
  void updateFromOccupancyGrid(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

  /**
   * @brief Update map data from ROS2 OccupancyGridUpdate message (incremental)
   *
   * @param msg ROS2 OccupancyGridUpdate message from /map_updates topic
   */
  void updateFromOccupancyGridUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg);

  /**
   * @brief Convert occupancy grid to QImage for rendering
   *
   * - Unknown (-1): RGB(128, 128, 128) gray
   * - Free (0): RGB(255, 255, 255) white
   * - Occupied (100): RGB(0, 0, 0) black
   * - Intermediate values: linear interpolation
   *
   * @return QImage in Format_RGB888 ready for QML rendering
   */
  QImage toQImage();

  /**
   * @brief Transform world coordinates to pixel coordinates
   *
   *
   * @param world_x X coordinate in world frame (meters)
   * @param world_y Y coordinate in world frame (meters)
   * @param pixel_x Output pixel X coordinate (modified)
   * @param pixel_y Output pixel Y coordinate (modified)
   *
   * @return true if coordinates are within map bounds, false otherwise
   */
  bool worldToPixel(double world_x, double world_y, int& pixel_x, int& pixel_y) const;

  /**
   * @brief Transform pixel coordinates to world coordinates
   *
   * @param pixel_x Pixel X coordinate
   * @param pixel_y Pixel Y coordinate
   * @param world_x Output world X coordinate in meters (modified)
   * @param world_y Output world Y coordinate in meters (modified)
   */
  void pixelToWorld(int pixel_x, int pixel_y, double& world_x, double& world_y) const;

  /**
   * @brief Get current map metadata
   *
   * @return Const reference to map metadata structure
   */
  const MapMetadata& getMetadata() const;

  /**
   * @brief Check if map data is available
   *
   * @return true if map has been initialized with valid data, false otherwise
   */
  bool isAvailable() const;

private:
  /**
   * @brief Regenerate QImage from occupancy data
   *
   */
  void regenerateImage();

  MapMetadata metadata_;               /**< Map resolution, dimensions and origin */
  std::vector<int8_t> occupancy_data_; /**< Occupancy grid data (row-major order) */
  QImage cached_image_;                /**< Cached converted image for performance */
  bool image_dirty_;                   /**< Flag indicating if image needs regeneration */
  bool is_available_;                  /**< Flag indicating if map data has been received */
};

} // namespace map
} // namespace ROBOGait
