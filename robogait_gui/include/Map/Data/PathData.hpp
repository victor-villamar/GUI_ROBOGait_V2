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
 * @brief Class for handling path data
 */
class PathData
{
public:
  /**
   * @brief Structure representing a single path point in map coordinates
   *
   * @param x The x-coordinate of the path point
   * @param y The y-coordinate of the path point
   */
  struct PathPoint
  {
    double x;
    double y;

    PathPoint() : x(0.0), y(0.0) {}
    PathPoint(double x, double y) : x(x), y(y) {}
  };

  /**
   * @brief Path metadata
   *
   * @param points The points in the path
   */
  struct PathMetadata
  {
    std::vector<PathPoint> points;
  };

  /**
   * @brief Constructor of PathData class
   */
  PathData();

  /**
   * @brief Destructor of PathData class
   */
  ~PathData() = default;

  /**
   * @brief Set the path data
   *
   * @param metadata Path metadata
   */
  void setPath(const PathMetadata& metadata);

  /**
   * @brief Get the metadata from the path data
   *
   * @return The metadata from the path data
   */
  PathMetadata getMetadata() const;

  /**
   * @brief Get the points from the path data
   *
   * @return The points from the path data
   */
  std::vector<PathPoint> getPoints() const;

  /**
   * @brief Check if the path data is available
   *
   * @return true if points are available, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Reset the path data
   */
  void reset();

  /**
   * @brief Get the timestamp of the last update
   *
   * @return The timestamp of the last update
   */
  uint64_t getUpdateStamp() const;

private:
  PathMetadata metadata_; /**< The metadata from the path data */
  bool is_available_;     /**< Flag indicating if the data is available */
  uint64_t update_stamp_; /**< The update timestamp */

  mutable std::mutex data_mutex_; /**< Mutex for protecting shared data */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
