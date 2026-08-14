#include <cmath>
#include <cstddef>

#include <QDebug>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Transform.h>
#include <tf2/exceptions.hpp>

#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "Map/Subscribers/LaserScanSubscriber.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::subscribers;

LaserScanSubscriber::LaserScanSubscriber() :
    parent_node_(nullptr),
    laser_scan_data_(nullptr),
    map_frame_(ROBOGait::ros::topics::TF_MAP_FRAME),
    active_(false),
    warn_logged_(false),
    context_(std::nullopt)
{
}

LaserScanSubscriber::~LaserScanSubscriber() { stop(); }

void LaserScanSubscriber::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[LaserScanSubscriber::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
}

void LaserScanSubscriber::setLaserScanData(data::LaserScanData* laser_scan_data) { laser_scan_data_ = laser_scan_data; }

void LaserScanSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;

  if (context_)
  {
    map_frame_ = context_->resolveFrame(map_frame_);
  }
}

void LaserScanSubscriber::start()
{
  if (!parent_node_)
  {
    qCritical() << "[LaserScanSubscriber::start] Parent node is null";
    return;
  }

  if (active_)
  {
    return;
  }

  // Create TF buffer and listener
  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(parent_node_->get_clock());
  tf_buffer_->setUsingDedicatedThread(true);
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_, parent_node_, false);

  std::string scan_topic = ROBOGait::ros::topics::T_SCAN;

  if (context_)
  {
    scan_topic = context_->resolveTopic(scan_topic);
  }

  sub_scan_ = parent_node_->create_subscription<sensor_msgs::msg::LaserScan>(scan_topic, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT(),
                                                                             std::bind(&LaserScanSubscriber::callbackScan, this, std::placeholders::_1));

  active_ = true;
  warn_logged_ = false;
  qDebug() << "[LaserScanSubscriber::start] Subscribed to laser scan topic:" << scan_topic.c_str();
}

void LaserScanSubscriber::stop()
{
  if (!active_)
  {
    return;
  }

  if (sub_scan_)
  {
    sub_scan_.reset();
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

  qDebug() << "[LaserScanSubscriber::stop] Subscriptions stopped";
}

bool LaserScanSubscriber::isActive() const { return active_; }

void LaserScanSubscriber::callbackScan(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
  if (!laser_scan_data_)
  {
    qCritical() << "[LaserScanSubscriber::callbackScan] LaserScanData is null";
    return;
  }

  if (!msg)
  {
    qWarning() << "[LaserScanSubscriber::callbackScan] Invalid LaserScan message";
    return;
  }

  if (!tf_buffer_)
  {
    qCritical() << "[LaserScanSubscriber::callbackScan] TF buffer is not initialized";
    return;
  }

  std::string scan_frame = msg->header.frame_id;

  if (!scan_frame.empty() && scan_frame.front() == '/')
  {
    scan_frame.erase(0, 1);
  }

  if (scan_frame.empty())
  {
    qWarning() << "[LaserScanSubscriber::callbackScan] Scan frame is empty";
    return;
  }

  data::LaserScanData::LaserScanMetadata metadata = transformLaserScan(msg, scan_frame);

  if (metadata.points.empty())
  {
    return;
  }

  laser_scan_data_->setLaserScanData(metadata);
  warn_logged_ = false;
}

ROBOGait::map::data::LaserScanData::LaserScanMetadata LaserScanSubscriber::transformLaserScan(const sensor_msgs::msg::LaserScan::SharedPtr msg,
                                                                                              const std::string& scan_frame)
{
  data::LaserScanData::LaserScanMetadata metadata;

  geometry_msgs::msg::TransformStamped transform_stamped;
  bool transform_found = false;

  // Look up the transform from the scan frame to the map frame
  try
  {
    transform_stamped = tf_buffer_->lookupTransform(map_frame_, scan_frame, msg->header.stamp);
    transform_found = true;
  }
  catch (const tf2::TransformException& ex)
  {
    try
    {
      transform_stamped = tf_buffer_->lookupTransform(map_frame_, scan_frame, tf2::TimePointZero);
      transform_found = true;
    }
    catch (const tf2::TransformException& e)
    {
      if (!warn_logged_)
      {
        qWarning() << "[LaserScanSubscriber::transformLaserScan] Could not transform from" << scan_frame.c_str() << "to" << map_frame_.c_str() << ":"
                   << e.what();
        warn_logged_ = true;
      }
      return metadata;
    }
  }

  if (!transform_found)
  {
    qWarning() << "[LaserScanSubscriber::transformLaserScan] Could not find transform from" << scan_frame.c_str() << "to" << map_frame_.c_str();
    return metadata;
  }

  tf2::Transform tf;
  tf2::fromMsg(transform_stamped.transform, tf);

  metadata.points.reserve(msg->ranges.size());

  double angle = static_cast<double>(msg->angle_min);
  for (size_t i = 0; i < msg->ranges.size(); ++i)
  {
    const float range = msg->ranges[i];
    if (!std::isfinite(range) || range < msg->range_min || range > msg->range_max)
    {
      angle += static_cast<double>(msg->angle_increment);
      continue;
    }

    // Convert polar coordinates to Cartesian coordinates
    const double lx = static_cast<double>(range) * cos(angle);
    const double ly = static_cast<double>(range) * sin(angle);

    // Transform to map frame
    const tf2::Vector3 laser_point(lx, ly, 0.0);
    const tf2::Vector3 map_point = tf * laser_point;

    metadata.points.emplace_back(map_point.x(), map_point.y());

    angle += static_cast<double>(msg->angle_increment);
  }

  return metadata;
}
