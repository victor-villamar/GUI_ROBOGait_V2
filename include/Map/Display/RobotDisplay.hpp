#pragma once

#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QMutex>

#include <memory>

#include "Map/Display/BaseDisplay.hpp"
#include "Map/RobotPose.hpp"

namespace ROBOGait
{
namespace map
{
namespace display
{

/**
 * @brief Display for robot pose visualization with smooth interpolation
 *
 * This display shows the robot's current pose on the map as:
 * - A circular body
 * - A directional arrow indicating orientation
 *
 * The display uses exponential interpolation for smooth movement:
 * - Position: alpha = 0.3 (slower, more damped)
 * - Rotation: alpha = 0.5 (faster response)
 */
class RobotDisplay : public BaseDisplay
{
  Q_OBJECT

public:
  /**
   * @brief Constructor
   * @param parent Qt parent object
   */
  explicit RobotDisplay(QObject* parent = nullptr);

  /**
   * @brief Destructor
   */
  ~RobotDisplay() override;

  /**
   * @brief Initialize display with parent ROS2 node
   * @param parent_node ROS2 node
   */
  void initialize(rclcpp::Node* parent_node) override;

  /**
   * @brief Shutdown display
   */
  void shutdown() override;

  /**
   * @brief Update display state with exponential interpolation
   *
   * @param wall_dt Wall time delta (seconds)
   * @param ros_dt ROS time delta (seconds)
   */
  void update(double wall_dt, double ros_dt) override;

  /**
   * @brief Get the graphics item group for the robot
   *
   * @return Graphics item group for the robot
   */
  QGraphicsItem* getGraphicsItem() override { return robot_group_.get(); }

  /**
   * @brief Set robot size (in meters)
   *
   * @param size Robot diameter in meters
   */
  void setRobotSize(double size);

  /**
   * @brief Get robot size
   *
   * @return Robot diameter in meters
   */
  double getRobotSize() const { return robot_size_; }

  /**
   * @brief Check if robot pose is available
   *
   * @return True if robot pose has been received from TF
   */
  bool isRobotPoseAvailable() const { return robot_pose_ && robot_pose_->isAvailable(); }

signals:
  void poseUpdated(); // Signal emitted when robot pose is updated  /*double x, double y, double yaw);*/

private:
  /**
   * @brief Create robot graphics items
   */
  void createRobotGraphics();

  /**
   * @brief Update robot graphics based on interpolated pose
   */
  void updateRobotGraphics();

  std::shared_ptr<ROBOGait::map::RobotPose> robot_pose_; /**< Robot pose tracker */

  std::unique_ptr<QGraphicsItemGroup> robot_group_; /**< Group containing all robot graphics */
  QGraphicsEllipseItem* body_item_;                 /**< Robot body circle */
  QGraphicsPolygonItem* arrow_item_;                /**< Directional arrow */

  // Interpolated pose
  double interpolated_x_;   /**< Interpolated X position (meters) */
  double interpolated_y_;   /**< Interpolated Y position (meters) */
  double interpolated_yaw_; /**< Interpolated yaw angle (radians) */

  // Interpolation parameters
  static constexpr double ALPHA_POSITION = 0.3; /**< Position interpolation factor */
  static constexpr double ALPHA_ROTATION = 0.5; /**< Rotation interpolation factor */

  // TODO: Obtain from Loader
  double robot_size_; /**< Robot size in meters (default: 0.5m diameter) */
  QMutex data_mutex_; /**< Mutex for thread-safe data access */
  bool first_update_; /**< Flag for first update (no interpolation) */
};

} // namespace display
} // namespace map
} // namespace ROBOGait
