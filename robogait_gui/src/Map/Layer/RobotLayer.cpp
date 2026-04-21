#include <QtMath>

#include "Map/Layer/RobotLayer.hpp"
#include "Map/Rendering/RenderPipeline.hpp"
#include "Ros/Define.hpp"

using namespace ROBOGait::map::layer;

RobotLayer::RobotLayer() :
    interpolated_pose_{0.0, 0.0, 0.0},
    target_pose_{0.0, 0.0, 0.0},
    first_update_(true),
    render_requested_(false),
    last_update_(),
    has_last_update_(false),
    robot_size_(DEFAULT_ROBOT_SIZE)
{
}

void RobotLayer::setRobotPoseData(std::shared_ptr<data::RobotPoseData> robot_pose_data)
{
  QMutexLocker lock(&state_mutex_);
  robot_pose_data_ = std::move(robot_pose_data);
}

RobotLayer::Pose2D RobotLayer::getInterpolatedPose() const
{
  QMutexLocker lock(&state_mutex_);
  return interpolated_pose_;
}

void RobotLayer::setRobotSize(double size)
{
  if (size > 0.0)
  {
    robot_size_ = size;
  }
}

double RobotLayer::getRobotSize() const { return robot_size_; }

void RobotLayer::update()
{
  if (!robot_pose_data_ || !robot_pose_data_->isAvailable())
  {
    return;
  }

  const RobotLayer::Pose2D current_target = readTargetPose();

  QMutexLocker lock(&state_mutex_);
  target_pose_ = current_target;

  const auto now = std::chrono::steady_clock::now();
  double wall_dt = 1.0 / REFERENCE_UPDATE_HZ;
  if (has_last_update_)
  {
    wall_dt = std::chrono::duration<double>(now - last_update_).count();
  }
  last_update_ = now;
  has_last_update_ = true;

  if (first_update_)
  {
    interpolated_pose_ = target_pose_;
    first_update_ = false;
    render_requested_ = true;
    return;
  }

  if (wall_dt <= 0.0)
  {
    wall_dt = 1.0 / REFERENCE_UPDATE_HZ;
  }

  double dt_factor = wall_dt * REFERENCE_UPDATE_HZ;

  // Normalize dt_factor to [0, 3]
  if (dt_factor > MAX_INTERPOLATION_FACTOR)
  {
    dt_factor = MAX_INTERPOLATION_FACTOR;
  }
  const double alpha_pos = 1.0 - std::pow(1.0 - ALPHA_POSITION, dt_factor);
  interpolated_pose_.x += (target_pose_.x - interpolated_pose_.x) * alpha_pos;
  interpolated_pose_.y += (target_pose_.y - interpolated_pose_.y) * alpha_pos;

  const double alpha_rot = 1.0 - std::pow(1.0 - ALPHA_ROTATION, dt_factor);
  double yaw_diff = target_pose_.yaw - interpolated_pose_.yaw;
  while (yaw_diff > M_PI)
  {
    yaw_diff -= 2.0 * M_PI;
  }
  while (yaw_diff < -M_PI)
  {
    yaw_diff += 2.0 * M_PI;
  }

  interpolated_pose_.yaw += yaw_diff * alpha_rot;

  while (interpolated_pose_.yaw > M_PI)
  {
    interpolated_pose_.yaw -= 2.0 * M_PI;
  }
  while (interpolated_pose_.yaw < -M_PI)
  {
    interpolated_pose_.yaw += 2.0 * M_PI;
  }

  render_requested_ = true;
}

bool RobotLayer::needsRender() const { return render_requested_; }

void RobotLayer::clearRenderRequest() { render_requested_ = false; }

void RobotLayer::resetInterpolation()
{
  QMutexLocker lock(&state_mutex_);
  first_update_ = true;
  has_last_update_ = false;
  render_requested_ = true;
}

RobotLayer::Pose2D RobotLayer::readTargetPose() const
{
  RobotLayer::Pose2D pose{0.0, 0.0, 0.0};

  if (!robot_pose_data_ || !robot_pose_data_->isAvailable())
  {
    return pose;
  }
  auto metadata = robot_pose_data_->getMetadata();
  pose.x = metadata.x;
  pose.y = metadata.y;
  pose.yaw = metadata.theta;
  return pose;
}
