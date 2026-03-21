#include <QDebug>
#include <QtMath>

#include "Robot/RobotPlacementController.hpp"

using namespace ROBOGait::robot;

RobotPlacementController::RobotPlacementController(QObject* parent) :
    QObject(parent), map_visualization_manager_(nullptr), has_position_(false), has_orientation_(false), position_(QPointF(0.0, 0.0)), theta_(0.0)
{
}

bool RobotPlacementController::hasPosition() const { return has_position_; }

bool RobotPlacementController::hasOrientation() const { return has_orientation_; }

bool RobotPlacementController::isComplete() const { return has_position_ && has_orientation_; }

QPointF RobotPlacementController::getPosition() const { return position_; }

void RobotPlacementController::setPosition(const QPointF& position)
{
  if (position != position_)
  {
    position_ = position;
    has_position_ = true;

    applyPose();
    emit poseChanged();
  }
}

double RobotPlacementController::getTheta() const { return theta_; }

void RobotPlacementController::setTheta(double theta)
{
  if (theta_ != theta)
  {
    theta_ = theta;
    has_orientation_ = true;

    applyPose();
    emit poseChanged();
  }
}

void RobotPlacementController::setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager) { map_visualization_manager_ = manager; }

void RobotPlacementController::clear()
{
  has_position_ = false;
  has_orientation_ = false;
  position_ = QPointF(0.0, 0.0);
  theta_ = 0.0;

  emit poseChanged();
}

void RobotPlacementController::setPositionFromScreenCoordinates(double screen_x, double screen_y)
{
  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[RobotPlacementController::setPositionFromScreenCoordinates] MapVisualizationManager not set, cannot convert screen coordinates";
    return;
  }

  QPointF map_point;

  const bool ok = map_visualization_manager_->screenToMap(QPointF(screen_x, screen_y), map_point);
  if (!ok)
  {
    qCritical() << "[RobotPlacementController::setPositionFromScreenCoordinates] Failed to convert screen coordinates to map coordinates";
    return;
  }

  if (!map_visualization_manager_->isMapPointInside(map_point.x(), map_point.y()))
  {
    qWarning() << "[RobotPlacementController::setPositionFromScreenCoordinates] Map point is outside of map boundaries, ignoring";
    return;
  }

  const QPointF ros_point(map_point.x(), -map_point.y());
  setPosition(ros_point);
}

void RobotPlacementController::setOrientationDegrees(double degrees) { setTheta(qDegreesToRadians(degrees)); }

QVariantMap RobotPlacementController::getPoseMap() const
{
  QVariantMap pose_map;
  pose_map["x"] = position_.x();
  pose_map["y"] = position_.y();
  pose_map["theta"] = theta_;
  pose_map["has_position"] = has_position_;
  pose_map["has_orientation"] = has_orientation_;
  return pose_map;
}

void ROBOGait::robot::RobotPlacementController::applyPose()
{
  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[RobotPlacementController::applyPose] Cannot apply pose becauseition MapVisualizationManager is not available";
    return;
  }

  if (!has_position_)
  {
    qCritical() << "[RobotPlacementController::applyPose] Cannot apply pose because position is not set";
    return;
  }

  map_visualization_manager_->setManualRobotPose(position_.x(), position_.y(), theta_);
}

bool RobotPlacementController::ensureMapVisualizationManager() const
{
  if (!map_visualization_manager_)
  {
    qCritical() << "[RobotPlacementController::ensureMapVisualizationManager] MapVisualizationManager pointer is null";
    return false;
  }
  return true;
}
