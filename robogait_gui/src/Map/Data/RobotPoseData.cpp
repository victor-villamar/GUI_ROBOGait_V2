#include <cmath>

#include <QDebug>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/exceptions.h>

#include "Map/Data/RobotPoseData.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/Define.hpp"

using namespace ROBOGait::map::data;

RobotPoseData::RobotPoseData(rclcpp::Node* parent_node, const std::string& map_frame, const std::string& robot_frame) :
    parent_node_(parent_node),
    map_frame_(map_frame),
    robot_frame_(robot_frame),
    x_(0.0),
    y_(0.0),
    theta_(0.0),
    is_available_(false),
    warn_logged_(false),
    update_stamp_(0),
    has_context_(false),
    enabled_(false)
{
}

RobotPoseData::~RobotPoseData()
{
  stopTFListener();

  qInfo() << "[RobotPoseData::~RobotPoseData] Robot pose data destroyed";
}

void RobotPoseData::updatePoseFromTF()
{
  std::shared_ptr<tf2_ros::Buffer> buffer;
  {
    QMutexLocker lock(&data_mutex_);
    if (!enabled_ || !tf_buffer_)
    {
      qWarning() << "[RobotPoseData::updatePoseFromTF] TF updates not enabled or buffer not available";
      return;
    }

    buffer = tf_buffer_;
  }

  try
  {
    // Lookup transform from map to robot base_link
    // tf2::TimePointZero gets the latest available transform
    geometry_msgs::msg::TransformStamped transform = buffer->lookupTransform(map_frame_, robot_frame_, tf2::TimePointZero);

    // Extract position
    {
      QMutexLocker lock(&data_mutex_);
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

      ++update_stamp_;
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

    QMutexLocker lock(&data_mutex_);
    is_available_ = false;
  }
  catch (const std::exception& ex)
  {
    qCritical() << "[RobotPoseData::updatePoseFromTF] Unexpected error:" << ex.what();
    QMutexLocker lock(&data_mutex_);
    is_available_ = false;
  }
}

double RobotPoseData::getX() const
{
  QMutexLocker lock(&data_mutex_);
  return x_;
}

double RobotPoseData::getY() const
{
  QMutexLocker lock(&data_mutex_);
  return y_;
}

double RobotPoseData::getTheta() const
{
  QMutexLocker lock(&data_mutex_);
  return theta_;
}

bool RobotPoseData::isAvailable() const
{
  QMutexLocker lock(&data_mutex_);
  return is_available_;
}

bool RobotPoseData::isEnabled() const
{
  QMutexLocker lock(&data_mutex_);
  return enabled_;
}

void RobotPoseData::reset()
{
  QMutexLocker lock(&data_mutex_);
  x_ = 0.0;
  y_ = 0.0;
  theta_ = 0.0;
  is_available_ = false;
  warn_logged_ = false;
  ++update_stamp_;

  qInfo() << "[RobotPoseData::reset] Robot pose reset to origin";
}

void RobotPoseData::startTFListener()
{
  if (!parent_node_)
  {
    qCritical() << "[RobotPoseData::startTFListener] Null parent node pointer";
    return;
  }

  // Create TF2 buffer and listener
  if (!tf_buffer_)
  {
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
  }
  if (!tf_listener_)
  {
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, parent_node_, false);
  }

  // Create timer for periodic TF updates
  if (!tf_timer_)
  {
    tf_timer_ = parent_node_->create_wall_timer(std::chrono::milliseconds(TIME_TO_ROBOT_POSE_UPDATE), std::bind(&RobotPoseData::updatePoseFromTF, this));
  }

  {
    QMutexLocker lock(&data_mutex_);
    enabled_ = true;
    is_available_ = false;
    warn_logged_ = false;
  }

  qInfo() << "[RobotPoseData::startTFListener] TF listener started for frames:" << QString::fromStdString(map_frame_) << "->"
          << QString::fromStdString(robot_frame_);
}

void RobotPoseData::stopTFListener()
{
  {
    QMutexLocker lock(&data_mutex_);
    enabled_ = false;
    is_available_ = false;
    warn_logged_ = false;
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

  qInfo() << "[RobotPoseData::stopTFListener] TF listener stopped";
}

void RobotPoseData::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;

  QMutexLocker lock(&data_mutex_);
  map_frame_ = context_.resolveFrame(map_frame_);
  robot_frame_ = context_.resolveFrame(robot_frame_);
}

uint64_t RobotPoseData::getUpdateStamp() const
{
  QMutexLocker lock(&data_mutex_);
  return update_stamp_;
}
