#pragma once

#include <stdint.h>
#include <vector>

#include <QImage>
#include <QMutex>

#include <map_msgs/msg/occupancy_grid_update.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>

#include "Context/RobotContext.hpp"

namespace ROBOGait
{
namespace map
{
namespace data
{

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
    double resolution;
    uint32_t width;
    uint32_t height;
    double origin_x;
    double origin_y;
    double origin_theta;

    MapMetadata() : resolution(0.05), width(0), height(0), origin_x(0.0), origin_y(0.0), origin_theta(0.0) {}
  };

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
   * @brief Get current map metadata
   *
   * @return Map metadata structure
   */
  MapMetadata getMetadata() const;

  /**
   * @brief Check if map data is available
   *
   * @return true if map has been initialized with valid data, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Reset map data to initial state
   */
  void reset();

  /**
   * @brief Get the data update stamp
   *
   * @return Monotonic counter incremented on each update
   */
  uint64_t getUpdateStamp() const;

  /**
   * @brief Provide robot context for topic resolution
   *
   * @param context Robot context with namespace info
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Check if a context has been provided
   *
   * @return true if context is set, false otherwise
   */
  bool hasRobotContext() const;

  /**
   * @brief Get the map topic using the stored context
   *
   * @return Fully qualified map topic name
   */
  std::string mapTopic() const;

  /**
   * @brief Resolve the map updates topic using the stored context
   *
   * @return Fully qualified map updates topic name
   */
  std::string mapUpdatesTopic() const;

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
  uint64_t update_stamp_;              /**< Monotonic update counter */

  mutable QMutex data_mutex_; /**< Protects map data and image */

  ROBOGait::context::RobotContext context_; /**< Robot context for topic resolution */
  bool has_context_;                        /**< Flag indicating if context is set */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
