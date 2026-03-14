#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ROBOGait
{
namespace map
{
namespace data
{
/**
 * @brief Class for handling laser scan data
 */
class LaserScanData
{

public:
  /**
   * @brief Structure representing a single laser point in Cartesian coordinates
   *
   * @param x The x-coordinate of the laser point
   * @param y The y-coordinate of the laser point
   */
  struct LaserPoint
  {
    double x;
    double y;

    LaserPoint() : x(0.0), y(0.0) {}
    LaserPoint(double x, double y) : x(x), y(y) {}
  };

  /**
   * @brief Laser Scan Metadata
   *
   * @param points The points detected in the laser scan
   */
  struct LaserScanMetadata
  {
    std::vector<LaserPoint> points;
  };

  /**
   * @brief Constructor of LaserScanData class
   */
  LaserScanData();

  /**
   * @brief Destructor of LaserScanData class
   */
  ~LaserScanData() = default;

  /**
   * @brief Set the laser scan data
   *
   * @param metadata Laser scan metadata
   */
  void setLaserScanData(const LaserScanMetadata& metadata);

  /**
   * @brief Get the metadata from the laser scan data
   *
   * @return The metadata from the laser scan data
   */
  LaserScanMetadata getMetadata() const;

  /**
   * @brief Get the points from the laser scan data
   *
   * @return The points from the laser scan data
   */
  std::vector<LaserPoint> getPoints() const;

  /**
   * @brief Check if the laser scan data is available
   *
   * @return true if points are available, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Reset the laser scan data
   */
  void reset();

  /**
   * @brief Get the timestamp of the last update
   *
   * @return The timestamp of the last update
   */
  uint64_t getUpdateStamp() const;

private:
  LaserScanMetadata metadata_; /**< The metadata from the laser scan data */
  bool is_available_;          /**< Flag indicating if the data is available */
  uint64_t update_stamp_;      /**< The update timestamp */

  mutable std::mutex data_mutex_; /**< Mutex for protecting shared data */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
