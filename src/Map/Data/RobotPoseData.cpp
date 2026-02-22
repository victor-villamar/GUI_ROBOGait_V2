#include <QDebug>
#include <cmath>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/exceptions.h>

#include "Map/Data/RobotPoseData.hpp"
#include "Map/Utils/Utils.hpp"

using namespace ROBOGait::map::data;

RobotPoseData::RobotPoseData(rclcpp::Node* parent_node, const std::string& map_frame, const std::string& robot_frame, double update_rate) :
    parent_node_(parent_node),
    map_frame_(map_frame),
    robot_frame_(robot_frame),
    x_(0.0),
    y_(0.0),
    theta_(0.0),
    is_available_(false),
    warn_logged_(false),
    has_context_(false)
{
  if (!parent_node_)
  {
    qCritical() << "[RobotPoseData::RobotPoseData] Null parent node pointer";
    return;
  }

  // Create TF2 buffer and listener
  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, parent_node_, false);

  // Create timer for periodic TF updates
  auto update_period = std::chrono::duration<double>(1.0 / update_rate);
  tf_timer_ =
      parent_node_->create_wall_timer(std::chrono::duration_cast<std::chrono::milliseconds>(update_period), std::bind(&RobotPoseData::updatePoseFromTF, this));
}

RobotPoseData::~RobotPoseData()
{
  if (tf_timer_)
  {
    tf_timer_->cancel();
  }

  qInfo() << "[RobotPoseData::~RobotPoseData] Robot pose destroyed";
}

void RobotPoseData::updatePoseFromTF()
{
  try
  {
    // Lookup transform from map to robot base_link
    // tf2::TimePointZero gets the latest available transform
    geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(map_frame_, robot_frame_, tf2::TimePointZero);

    // Extract position
    x_ = transform.transform.translation.x;
    y_ = transform.transform.translation.y;

    // Extract orientation (convert quaternion to yaw)
    theta_ = utils::getYaw(transform.transform.rotation);

    if (!is_available_)
    {
      qInfo() << "[RobotPoseData::updatePoseFromTF] Robot pose now available from TF";
      is_available_ = true;
      warn_logged_ = false;
    }
  }
  catch (const tf2::TransformException& ex)
  {
    // Robot not localized yet or TF not available
    if (!warn_logged_)
    {
      qWarning() << "[RobotPoseData::updatePoseFromTF] Could not get transform from" << QString::fromStdString(map_frame_) << "to"
                 << QString::fromStdString(robot_frame_) << ":" << ex.what();
      qWarning() << "[RobotPoseData::updatePoseFromTF] Waiting for robot localization...";
      warn_logged_ = true;
    }

    is_available_ = false;
  }
  catch (const std::exception& ex)
  {
    qCritical() << "[RobotPoseData::updatePoseFromTF] Unexpected error:" << ex.what();
    is_available_ = false;
  }
}

double RobotPoseData::getX() const { return x_; }

double RobotPoseData::getY() const { return y_; }

double RobotPoseData::getTheta() const { return theta_; }

bool RobotPoseData::isAvailable() const { return is_available_; }

void RobotPoseData::reset()
{
  x_ = 0.0;
  y_ = 0.0;
  theta_ = 0.0;
  is_available_ = false;
  warn_logged_ = false;

  qInfo() << "[RobotPoseData::reset] Robot pose reset to origin";
}

void RobotPoseData::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;

  map_frame_ = context_.resolveFrame(map_frame_);
  robot_frame_ = context_.resolveFrame(robot_frame_);
}
