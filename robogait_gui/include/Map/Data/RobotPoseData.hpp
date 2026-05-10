#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace ROBOGait
{
namespace map
{
namespace data
{

/**
 * @brief Class to store and manage robot pose (position and orientation)
 */
class RobotPoseData
{
public:
  /**
   * @brief Robot Pose Metadata
   *
   * @param x_ Robot X position in meters
   * @param y_ Robot Y position in meters
   * @param theta_ Robot orientation in radians
   */
  struct RobotPoseMetadata
  {
    double x_;
    double y_;
    double theta_;

    RobotPoseMetadata() : x_(0.0), y_(0.0), theta_(0.0) {}
  };

  /**
   * @brief Constructor of RobotPoseData class
   */
  RobotPoseData();

  /**
   * @brief Destructor of RobotPoseData class
   */
  ~RobotPoseData() = default;

  /**
   * @brief Set robot pose metadata
   *
   * @param metadata Robot pose metadata
   */
  void setPose(const RobotPoseMetadata& metadata);

  /**
   * @brief Get robot pose metadata
   *
   * @return Robot pose metadata
   */
  RobotPoseMetadata getMetadata() const;

  /**
   * @brief Check if robot pose has been initialized
   *
   * @return true if pose data is available, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Reset pose to default values (origin)
   */
  void reset();

  /**
   * @brief Get the data update stamp
   *
   * @return Monotonic counter incremented on pose updates
   */
  uint64_t getUpdateStamp() const;

private:
  RobotPoseMetadata metadata_; /**< Robot pose metadata */
  bool is_available_;          /**< Flag indicating if pose has been received */
  uint64_t update_stamp_;      /**< Monotonic update counter */

  mutable std::mutex data_mutex_; /**< Protects pose data */
};

} // namespace data
} // namespace map
} // namespace ROBOGait
