#include <QBrush>
#include <QDebug>
#include <QPen>
#include <QtMath>

#include "Map/Display/RobotDisplay.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::display;

RobotDisplay::RobotDisplay(QObject* parent) :
    BaseDisplay("RobotDisplay", parent),
    robot_pose_(nullptr),
    body_item_(nullptr),
    arrow_item_(nullptr),
    interpolated_x_(0.0),
    interpolated_y_(0.0),
    interpolated_yaw_(0.0),
    robot_size_(0.5), // TODO: Obtain from Loader
    first_update_(true)
{
  // Create graphics items
  robot_group_ = std::make_unique<QGraphicsItemGroup>();
  createRobotGraphics();

  qInfo() << "[RobotDisplay::RobotDisplay] Robot display created";
}

RobotDisplay::~RobotDisplay() { shutdown(); }

void RobotDisplay::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[RobotDisplay::initialize] Null node pointer";
    return;
  }

  parent_node_ = parent_node;

  robot_pose_ = std::make_shared<ROBOGait::map::RobotPose>(parent_node_, std::string(TF_MAP_FRAME), std::string(TF_ROBOT_FRAME));
}

void RobotDisplay::shutdown()
{
  if (robot_pose_)
  {
    robot_pose_.reset();
  }

  qInfo() << "[RobotDisplay::shutdown] Robot display shutdown";
}

void RobotDisplay::update(double wall_dt, double ros_dt)
{
  (void)ros_dt; // Unused

  QMutexLocker lock(&data_mutex_);

  if (!robot_pose_ || !robot_pose_->isAvailable())
  {
    return;
  }

  // Get target pose from RobotPose
  double target_x = robot_pose_->getX();
  double target_y = robot_pose_->getY();
  double target_yaw = robot_pose_->getTheta();

  // First update: no interpolation, jump to target
  if (first_update_)
  {
    interpolated_x_ = target_x;
    interpolated_y_ = target_y;
    interpolated_yaw_ = target_yaw;
    first_update_ = false;

    // DEBUG
    // qDebug() << "[RobotDisplay::update] First pose update:" << interpolated_x_ << "," << interpolated_y_ << "Yaw:" << qRadiansToDegrees(interpolated_yaw_)
    //  << "deg";
  }
  else
  {
    double dt_factor = wall_dt * 30.0; // Normalize to 30Hz (dt = 1.0 at 30Hz)

    // Position interpolation (slower, alpha = 0.3)
    double alpha_pos = 1.0 - std::pow(1.0 - ALPHA_POSITION, dt_factor);
    interpolated_x_ += (target_x - interpolated_x_) * alpha_pos;
    interpolated_y_ += (target_y - interpolated_y_) * alpha_pos;

    // Rotation interpolation (faster, alpha = 0.5)
    double alpha_rot = 1.0 - std::pow(1.0 - ALPHA_ROTATION, dt_factor);

    // Handle angle wrap-around for shortest rotation path
    double yaw_diff = target_yaw - interpolated_yaw_;
    while (yaw_diff > M_PI)
    {
      yaw_diff -= 2.0 * M_PI;
    }
    while (yaw_diff < -M_PI)
    {
      yaw_diff += 2.0 * M_PI;
    }

    interpolated_yaw_ += yaw_diff * alpha_rot;

    // Normalize angle to [-pi, pi]
    while (interpolated_yaw_ > M_PI)
    {
      interpolated_yaw_ -= 2.0 * M_PI;
    }
    while (interpolated_yaw_ < -M_PI)
    {
      interpolated_yaw_ += 2.0 * M_PI;
    }
  }

  // Update graphics
  updateRobotGraphics();

  emit poseUpdated();
  emit renderRequested();
}

void RobotDisplay::setRobotSize(double size)
{
  if (size > 0.0 && qAbs(size - robot_size_) > 0.001)
  {
    robot_size_ = size;
    createRobotGraphics();
  }
}

void RobotDisplay::createRobotGraphics()
{
  // Clear existing items
  if (body_item_)
  {
    robot_group_->removeFromGroup(body_item_);
    delete body_item_;
  }
  if (arrow_item_)
  {
    robot_group_->removeFromGroup(arrow_item_);
    delete arrow_item_;
  }

  // Create body
  double radius = robot_size_ / 2.0;
  body_item_ = new QGraphicsEllipseItem(-radius, -radius, robot_size_, robot_size_);
  body_item_->setBrush(QBrush(QColor(0, 120, 255, 180))); // Semi-transparent blue
  body_item_->setPen(QPen(QColor(0, 80, 200), 0.02));     // Thin border (0.02m)
  robot_group_->addToGroup(body_item_);

  // Create arrow for direction
  QPolygonF arrow;
  double arrow_length = robot_size_ * 0.6;
  double arrow_width = robot_size_ * 0.3;

  arrow << QPointF(arrow_length, 0.0)        // Tip
        << QPointF(0.0, arrow_width / 2.0)   // Base right
        << QPointF(0.0, -arrow_width / 2.0); // Base left

  arrow_item_ = new QGraphicsPolygonItem(arrow);
  arrow_item_->setBrush(QBrush(QColor(255, 255, 0, 220))); // Yellow
  arrow_item_->setPen(QPen(QColor(200, 200, 0), 0.01));
  robot_group_->addToGroup(arrow_item_);
}

void RobotDisplay::updateRobotGraphics()
{
  if (!robot_group_)
  {
    return;
  }

  double scene_x = interpolated_x_;
  double scene_y = -interpolated_y_; // Flip Y axis, more accurate with ROS

  robot_group_->setPos(scene_x, scene_y);

  double qt_rotation = -qRadiansToDegrees(interpolated_yaw_); // Flip rotation, more accurate with ROS

  robot_group_->setRotation(qt_rotation);
}
