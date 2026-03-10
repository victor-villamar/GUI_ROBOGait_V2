#include <cmath>

#include <QDebug>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Transform.h>
#include <tf2/exceptions.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "Map/Data/LaserScanData.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::data;

LaserScanData::LaserScanData() :
    parent_node_(nullptr), is_available_(false), warn_logged_(false), update_stamp_(0), enabled_(false), map_frame_(TF_MAP_FRAME), has_context_(false)
{
  qInfo() << "[LaserScanData::LaserScanData] Laser scan data handler initialized";
}

LaserScanData::~LaserScanData() { stopTFListener(); }

void LaserScanData::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[LaserScanData::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
}

void LaserScanData::startTFListener()
{
  if (!parent_node_)
  {
    qCritical() << "[LaserScanData::startTFListener] Null parent node pointer";
    return;
  }

  if (!tf_buffer_)
  {
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
    tf_buffer_->setUsingDedicatedThread(true);
  }

  if (!tf_listener_)
  {
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, parent_node_, false);
  }

  QMutexLocker lock(&data_mutex_);
  enabled_ = true;
  warn_logged_ = false;
}

void LaserScanData::stopTFListener()
{
  {
    QMutexLocker lock(&data_mutex_);
    enabled_ = false;
    is_available_ = false;
    warn_logged_ = false;
  }

  if (tf_listener_)
  {
    tf_listener_.reset();
  }

  if (tf_buffer_)
  {
    tf_buffer_.reset();
  }
}

void LaserScanData::updateFromLaserScan(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
  if (!msg)
  {
    qCritical() << "[LaserScanData::updateFromLaserScan] Received null message";
    return;
  }

  std::shared_ptr<tf2_ros::Buffer> buffer;
  {
    QMutexLocker lock(&data_mutex_);

    if (!enabled_ || !tf_buffer_)
    {
      qCritical() << "[LaserScanData::updateFromLaserScan] TF listener not enabled or not initialized";
      return;
    }

    buffer = tf_buffer_;
  }

  std::string scan_frame = msg->header.frame_id;

  if (!scan_frame.empty() && scan_frame.front() == '/')
  {
    scan_frame.erase(0, 1);
  }

  if (scan_frame.empty())
  {
    qWarning() << "[LaserScanData::updateFromLaserScan] Empty scan frame id";
    return;
  }

  geometry_msgs::msg::TransformStamped transform;
  bool transform_ok = false;

  try
  {
    transform = buffer->lookupTransform(map_frame_, scan_frame, msg->header.stamp);
    transform_ok = true;
  }
  catch (const tf2::TransformException& ex)
  {
    try
    {
      transform = buffer->lookupTransform(map_frame_, scan_frame, tf2::TimePointZero);
      transform_ok = true;
    }
    catch (const tf2::TransformException& ex2)
    {
      QMutexLocker lock(&data_mutex_);

      if (!warn_logged_)
      {
        qWarning() << "[LaserScanData::updateFromLaserScan] Could not transform from" << QString::fromStdString(scan_frame) << "to"
                   << QString::fromStdString(map_frame_) << ":" << ex2.what();
        warn_logged_ = true;
      }

      points_.clear();
      is_available_ = false;
      ++update_stamp_;
      return;
    }
  }

  if (!transform_ok)
  {
    qCritical() << "[LaserScanData::updateFromLaserScan] Could not find transform from" << QString::fromStdString(scan_frame) << "to"
                << QString::fromStdString(map_frame_);
    return;
  }

  tf2::Transform tf;
  tf2::fromMsg(transform.transform, tf);

  std::vector<QPointF> points;
  points.reserve(msg->ranges.size());

  double angle = msg->angle_min;
  for (size_t i = 0; i < msg->ranges.size(); ++i)
  {
    const float range = msg->ranges[i];
    if (!std::isfinite(range) || range < msg->range_min || range > msg->range_max)
    {
      angle += msg->angle_increment;
      continue;
    }

    const double lx = range * std::cos(angle);
    const double ly = range * std::sin(angle);

    const tf2::Vector3 laser_point(lx, ly, 0.0);
    const tf2::Vector3 map_point = tf * laser_point;

    points.emplace_back(map_point.x(), map_point.y());

    angle += msg->angle_increment;
  }

  {
    QMutexLocker lock(&data_mutex_);
    points_ = std::move(points);
    is_available_ = true;
    warn_logged_ = false;
    ++update_stamp_;
  }
}

bool LaserScanData::getPoints(std::vector<QPointF>& points) const
{
  QMutexLocker lock(&data_mutex_);

  if (!is_available_)
  {
    return false;
  }

  points = points_;
  return true;
}

bool LaserScanData::isAvailable() const
{
  QMutexLocker lock(&data_mutex_);
  return is_available_;
}

uint64_t LaserScanData::getUpdateStamp() const
{
  QMutexLocker lock(&data_mutex_);
  return update_stamp_;
}

void LaserScanData::reset()
{
  QMutexLocker lock(&data_mutex_);
  points_.clear();
  is_available_ = false;
  warn_logged_ = false;
  ++update_stamp_;
}

void LaserScanData::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;

  QMutexLocker lock(&data_mutex_);
  map_frame_ = context_.resolveFrame(map_frame_);
}

bool LaserScanData::hasRobotContext() const
{
  QMutexLocker lock(&data_mutex_);
  return has_context_;
}

std::string LaserScanData::getScanTopic() const
{
  QMutexLocker lock(&data_mutex_);
  const std::string base = std::string(T_SCAN);
  if (!has_context_)
  {
    return base;
  }

  return context_.resolveTopic(base);
}
