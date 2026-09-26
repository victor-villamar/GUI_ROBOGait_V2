#include <QDebug>

#include <rclcpp/create_timer.hpp>
#include <tf2/exceptions.h>

#include <geometry_msgs/msg/transform_stamped.hpp>

#include "Map/Subscribers/TFSubscriber.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/Define.hpp"

using namespace ROBOGait::map::subscribers;

TFSubscriber::TFSubscriber() :
    parent_node_(nullptr), robot_pose_data_(nullptr), map_frame_(""), robot_frame_(""), active_(false), warn_logged_(false), paused_(false)
{
}

TFSubscriber::~TFSubscriber() { stop(); }

void TFSubscriber::initialize(rclcpp::Node* parent_node, const std::string& map_frame, const std::string& robot_frame)
{
  if (!parent_node)
  {
    qCritical() << "[TFSubscriber::initialize] Parent node is null";
    return;
  }

  parent_node_ = parent_node;
  map_frame_ = map_frame;
  robot_frame_ = robot_frame;
}

void TFSubscriber::setRobotPoseData(data::RobotPoseData* robot_pose_data) { robot_pose_data_ = robot_pose_data; }

void TFSubscriber::setTFBuffer(const std::shared_ptr<tf2_ros::Buffer>& tf_buffer) { tf_buffer_ = tf_buffer; }

void TFSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;

  if (context_)
  {
    map_frame_ = context_->resolveFrame(map_frame_);
    robot_frame_ = context_->resolveFrame(robot_frame_);
  }
}

void TFSubscriber::start()
{
  if (!parent_node_)
  {
    qCritical() << "[TFSubscriber::start] Parent node is null";
    return;
  }

  if (!robot_pose_data_)
  {
    qCritical() << "[TFSubscriber::start] Robot pose data is null";
    return;
  }

  if (!tf_buffer_)
  {
    qCritical() << "[TFSubscriber::start] Shared TF buffer is not set";
    return;
  }

  if (active_)
  {
    return;
  }

  tf_timer_ = parent_node_->create_wall_timer(ROBOGait::ros::define::TIME_TO_ROBOT_POSE_UPDATE_MS, std::bind(&TFSubscriber::updatePoseFromTF, this));

  active_ = true;
  warn_logged_ = false;

  qDebug() << "[TFSubscriber::start] TF pose updates started for frames:" << map_frame_.c_str() << "->" << robot_frame_.c_str();
}

void TFSubscriber::stop()
{
  const bool was_active = active_;

  if (tf_timer_)
  {
    tf_timer_->cancel();
    tf_timer_.reset();
  }

  tf_buffer_.reset();
  active_ = false;

  if (was_active)
  {
    qDebug() << "[TFSubscriber::stop] TF pose updates stopped";
  }
}

bool TFSubscriber::isActive() const { return active_; }

void TFSubscriber::setPaused(bool paused) { paused_ = paused; }

void TFSubscriber::updatePoseFromTF()
{
  if (paused_)
  {
    return;
  }

  if (!tf_buffer_ || !robot_pose_data_)
  {
    return;
  }

  // Lookup transform from map to robot base_link
  // tf2::TimePointZero gets the latest available transform
  try
  {
    geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(map_frame_, robot_frame_, tf2::TimePointZero);

    data::RobotPoseData::RobotPoseMetadata metadata;
    // Extract position
    metadata.x_ = transform.transform.translation.x;
    metadata.y_ = transform.transform.translation.y;

    // Extract orientation (convert quaternion to yaw)
    metadata.theta_ = ROBOGait::map::utils::getYaw(transform.transform.rotation);

    robot_pose_data_->setPose(metadata);

    warn_logged_ = false;
  }
  catch (const tf2::TransformException& e)
  {
    if (!warn_logged_)
    {
      qDebug() << "[TFSubscriber::updatePoseFromTF] Could not get transform from" << map_frame_.c_str() << "to" << robot_frame_.c_str() << ":" << e.what();
      qDebug() << "[TFSubscriber::updatePoseFromTF] Waiting for robot localization...";
      warn_logged_ = true;
    }
  }
  catch (const std::exception& e)
  {
    qCritical() << "[TFSubscriber::updatePoseFromTF] Unexpected error:" << e.what();
  }
}