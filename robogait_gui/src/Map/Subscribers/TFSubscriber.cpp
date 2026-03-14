#include <iostream>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <rclcpp/create_timer.hpp>
#include <tf2/exceptions.h>

#include "Map/Subscribers/TFSubscriber.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/Define.hpp"

using namespace ROBOGait::map::data;

TFSubscriber::TFSubscriber() : robot_pose_data_(nullptr), map_frame_(""), robot_frame_(""), active_(false), warn_logged_(false) {}

TFSubscriber::~TFSubscriber() { stop(); }

void TFSubscriber::initialize(rclcpp::Node* parent_node, const std::string& map_frame, const std::string& robot_frame)
{
  if (!parent_node)
  {
    std::cerr << "[TFSubscriber::initialize] Parent node is null" << std::endl;
    return;
  }

  parent_node_ = parent_node;
  map_frame_ = map_frame;
  robot_frame_ = robot_frame;
}

void TFSubscriber::setRobotPoseData(data::RobotPoseData* robot_pose_data) { robot_pose_data_ = robot_pose_data; }

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
    std::cerr << "[TFSubscriber::start] Parent node is null" << std::endl;
    return;
  }

  if (!robot_pose_data_)
  {
    std::cerr << "[TFSubscriber::start] Robot pose data is null" << std::endl;
    return;
  }

  if (active_)
  {
    return;
  }

  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, parent_node_, false);
  tf_timer_ = parent_node_->create_wall_timer(std::chrono::milliseconds(TIME_TO_ROBOT_POSE_UPDATE), std::bind(&TFSubscriber::updatePoseFromTF, this));

  active_ = true;
  warn_logged_ = false;

  std::cout << "[TFSubscriber::start] TF listener started for frames: " << map_frame_ << "->" << robot_frame_ << std::endl;
}

void TFSubscriber::stop()
{
  if (!active_)
  {
    return;
  }

  if (tf_timer_)
  {
    tf_timer_->cancel();
    tf_timer_.reset();
  }

  if (tf_listener_)
  {
    tf_listener_.reset();
  }

  if (tf_buffer_)
  {
    tf_buffer_.reset();
  }

  active_ = false;

  std::cout << "[TFSubscriber::stop] TF listener stopped" << std::endl;
}

bool TFSubscriber::isActive() const { return active_; }

void TFSubscriber::updatePoseFromTF()
{
  if (!tf_buffer_ || !robot_pose_data_)
  {
    return;
  }

  // Lookup transform from map to robot base_link
  // tf2::TimePointZero gets the latest available transform
  try
  {
    geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(map_frame_, robot_frame_, tf2::TimePointZero);

    RobotPoseData::RobotPoseMetadata metadata;
    // Extract position
    metadata.x = transform.transform.translation.x;
    metadata.y = transform.transform.translation.y;

    // Extract orientation (convert quaternion to yaw)
    metadata.theta = ROBOGait::map::utils::getYaw(transform.transform.rotation);

    robot_pose_data_->setPose(metadata);

    warn_logged_ = false;
  }
  catch (const tf2::TransformException& e)
  {
    if (!warn_logged_)
    {
      std::cerr << "[TFSubscriber::updatePoseFromTF] Could not get transform from " << map_frame_ << " to " << robot_frame_ << ": " << e.what() << std::endl;
      std::cerr << "[TFSubscriber::updatePoseFromTF] Waiting for robot localization..." << std::endl;
      warn_logged_ = true;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "[TFSubscriber::updatePoseFromTF] Unexpected error: " << e.what() << std::endl;
  }
}
