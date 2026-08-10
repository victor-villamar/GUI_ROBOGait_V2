#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

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
    double resolution_;
    uint32_t width_;
    uint32_t height_;
    double origin_x_;
    double origin_y_;
    double origin_theta_;

    MapMetadata() : resolution_(0.05), width_(0), height_(0), origin_x_(0.0), origin_y_(0.0), origin_theta_(0.0) {}
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
   * @brief Get current map metadata
   *
   * @return Map metadata structure
   */
  MapMetadata getMetadata() const;

  /**
   * @brief Set occupancy data for the map
   *
   * @param occupancy_data Occupancy data vector (row-major order)
   * @param metadata Map metadata (resolution, dimensions, origin)
   */
  void setOccupancyData(const std::vector<int8_t>& occupancy_data, const MapMetadata& metadata);

  /**
   * @brief Get occupancy data for the map
   *
   * @return Occupancy data vector (row-major order)
   */
  const std::vector<int8_t>& getOccupancyData() const;

  /**
   * @brief Update a region of the map with new occupancy data
   *
   * @param x X coordinate of the region (in cells)
   * @param y Y coordinate of the region (in cells)
   * @param width Width of the region (in cells)
   * @param height Height of the region (in cells)
   * @param data Region data (size must be equal to width * height)
   */
  void updateRegion(int32_t x, int32_t y, uint32_t width, uint32_t height, const std::vector<int8_t>& data);

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

private:
  MapMetadata metadata_;               /**< Map resolution, dimensions and origin */
  std::vector<int8_t> occupancy_data_; /**< Occupancy grid data (row-major order) */
  bool is_available_;                  /**< Flag indicating if map data has been received */
  uint64_t update_stamp_;              /**< Monotonic update counter */

  mutable std::mutex data_mutex_; /**< Protects map data and image */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
