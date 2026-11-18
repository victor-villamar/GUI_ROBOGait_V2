#pragma once

#include <chrono>
#include <memory>

#include <QMutex>

#include "Map/Data/RobotPoseData.hpp"
#include "Map/Layer/LayerInterface.hpp"

namespace ROBOGait
{
namespace map
{
namespace layer
{

/**
 * @brief Layer that manages the robot's pose updates and rendering
 */
class RobotLayer : public LayerInterface
{
public:
  /**
   * @brief 2D pose representation
   *
   * @param x X coordinate in map frame
   * @param y Y coordinate in map frame
   * @param yaw Yaw angle in radians
   */
  struct Pose2D
  {
    double x;
    double y;
    double yaw;
  };

  /**
   * @brief Constructor of RobotLayer class
   */
  explicit RobotLayer();

  /**
   * @brief Set the robot pose data
   *
   * @param robot_pose_data Shared pointer to the robot pose data
   */
  void setRobotPoseData(std::shared_ptr<data::RobotPoseData> robot_pose_data);

  /**
   * @brief Get the interpolated pose
   *
   * @return Interpolated pose
   */
  Pose2D getInterpolatedPose() const;

  /**
   * @brief Set the robot size
   *
   * @param size Robot size
   */
  void setRobotSize(double size);

  /**
   * @brief Get the robot size
   *
   * @return Robot size
   */
  double getRobotSize() const;

  /**
   * @brief Update the renderer state
   *
   * This function updates the state of the robot layer by interpolating the target pose
   * and requesting a render update if necessary.
   */
  void update() override;

  /**
   * @brief Check if the layer needs rendering
   *
   * @return True if rendering is needed, false otherwise
   */
  bool needsRender() const override;

  /**
   * @brief Clear the render request flag
   */
  void clearRenderRequest() override;

  /**
   * @brief Reset interpolation state so the next pose is applied immediately
   */
  void resetInterpolation();

private:
  /**
   * @brief Read the target pose from the data source
   *
   * @return Target pose
   */
  Pose2D readTargetPose() const;

  std::shared_ptr<data::RobotPoseData> robot_pose_data_; /**< Robot pose data source */

  mutable QMutex state_mutex_; /**< Mutex for protecting state access */
  Pose2D interpolated_pose_;   /**< Interpolated pose */
  Pose2D target_pose_;         /**< Target pose */

  bool first_update_;                                 /**< Flag for first update */
  bool render_requested_;                             /**< Flag for render request */
  std::chrono::steady_clock::time_point last_update_; /**< Last update time */
  bool has_last_update_;                              /**< Flag for last update availability */
  double robot_size_;                                 /**< Robot size for rendering */

  static constexpr double ALPHA_POSITION = 0.3;           /**< Position interpolation factor */
  static constexpr double ALPHA_ROTATION = 0.5;           /**< Rotation interpolation factor */
  static constexpr double REFERENCE_UPDATE_HZ = 30.0;     /**< Reference update frequency for interpolation */
  static constexpr double MAX_INTERPOLATION_FACTOR = 3.0; /**< Maximum interpolation factor to prevent overshooting */
  static constexpr double DEFAULT_ROBOT_SIZE = 0.5;       /**< Default robot size in meters */
};

} // namespace layer
} // namespace map
} // namespace ROBOGait
