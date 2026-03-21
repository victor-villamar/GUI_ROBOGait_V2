#pragma once

#include <QObject>
#include <QPointF>
#include <QVariantMap>

#include "Map/MapVisualizationManager.hpp"

namespace ROBOGait
{
namespace robot
{
/**
 * @brief Controller for managing robot placement on the map
 */
class RobotPlacementController : public QObject
{
  Q_OBJECT
public:
  // clang-format off
  Q_PROPERTY(bool hasPosition
             READ hasPosition
             NOTIFY poseChanged)
  
  Q_PROPERTY(bool hasOrientation
             READ hasOrientation
             NOTIFY poseChanged)

  Q_PROPERTY(bool isComplete
           READ isComplete
           NOTIFY poseChanged)

  Q_PROPERTY(QPointF position
             READ getPosition
             WRITE setPosition
             NOTIFY poseChanged)
  Q_PROPERTY(double theta
             READ getTheta
             WRITE setTheta
             NOTIFY poseChanged)
  // clang-format on

  /**
   * @brief Constructor of RobotPlacementController class
   */
  explicit RobotPlacementController(QObject* parent = nullptr);

  /**
   * @brief Check if position has been set
   *
   * @return true if position is set, false otherwise
   */
  bool hasPosition() const;

  /**
   * @brief Check if orientation has been set
   *
   * @return true if orientation is set, false otherwise
   */
  bool hasOrientation() const;

  /**
   * @brief Check if both position and orientation have been set
   *
   * @return true if both position and orientation are set, false otherwise
   */
  bool isComplete() const;

  /**
   * @brief Get the robot's position
   *
   * @return Robot position as QPointF (x, y)
   */
  QPointF getPosition() const;

  /**
   * @brief Set the robot's position
   *
   * @param position Robot position as QPointF (x, y)
   */
  void setPosition(const QPointF& position);

  /**
   * @brief Get the robot's orientation
   *
   * @return Robot orientation in radians (0 = facing right, positive counter-clockwise)
   */
  double getTheta() const;

  /**
   * @brief Set the robot's orientation
   *
   * @param theta Robot orientation in radians (0 = facing right, positive counter-clockwise)
   */
  void setTheta(double theta);

  /**
   * @brief Set the MapVisualizationManager instance for coordinate conversions
   *
   * @param manager Pointer to the MapVisualizationManager instance
   */
  void setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager);

  /**
   * @brief Clear the robot's position and orientation
   */
  Q_INVOKABLE void clear();

  /**
   * @brief Set the robot's position from screen coordinates
   *
   * @param screen_x Screen x-coordinate
   * @param screen_y Screen y-coordinate
   */
  Q_INVOKABLE void setPositionFromScreenCoordinates(double screen_x, double screen_y);

  /**
   * @brief Set the robot's orientation in degrees
   *
   * @param degrees Orientation in degrees (0 = facing right, positive counter-clockwise)
   */
  Q_INVOKABLE void setOrientationDegrees(double degrees);

  /**
   * @brief Get the robot's pose as a QVariantMap
   *
   * @return Robot pose as a QVariantMap
   */
  Q_INVOKABLE QVariantMap getPoseMap() const;

signals:
  void poseChanged(); // Emitted when position or orientation changes

private:
  /**
   * @brief Apply the current pose to the MapVisualizationManager
   */
  void applyPose();

  /**
   * @brief Ensure that the MapVisualizationManager is set before performing operations that require it
   *
   * @return true if MapVisualizationManager is set, false otherwise
   */
  bool ensureMapVisualizationManager() const;

  ROBOGait::map::manager::MapVisualizationManager* map_visualization_manager_; /**< Pointer to MapVisualizationManager */
  bool has_position_;                                                          /**< Flag indicating if position is set */
  bool has_orientation_;                                                       /**< Flag indicating if orientation is set */
  QPointF position_;                                                           /**< Robot position (x, y) in map coordinates */
  double theta_;                                                               /**< Robot orientation (theta) in radians */
};
} // namespace robot
} // namespace ROBOGait