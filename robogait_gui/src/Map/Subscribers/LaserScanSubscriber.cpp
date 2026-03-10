#include <QDebug>

#include "Map/Subscribers/LaserScanSubscriber.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::data;

LaserScanSubscriber::LaserScanSubscriber() : parent_node_(nullptr), has_context_(false), active_(false) {}

void LaserScanSubscriber::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[LaserScanSubscriber::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;
}

void LaserScanSubscriber::setScanData(const std::shared_ptr<LaserScanData>& scan_data)
{
  scan_data_ = scan_data;
  if (scan_data_ && has_context_)
  {
    scan_data_->setRobotContext(context_);
  }
}

void LaserScanSubscriber::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;
  if (scan_data_)
  {
    scan_data_->setRobotContext(context_);
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

  const std::string scan_topic = scan_data_ ? scan_data_->getScanTopic() : std::string(T_SCAN);

  sub_scan_ = parent_node_->create_subscription<sensor_msgs::msg::LaserScan>(scan_topic, QOS_BEST_EFFORT.keep_last(5),
                                                                             std::bind(&LaserScanSubscriber::callbackScan, this, std::placeholders::_1));

  active_ = true;
  qInfo() << "[LaserScanSubscriber::start] Subscribed to laser scan topic:" << QString::fromStdString(scan_topic);
}

void LaserScanSubscriber::stop()
{
  if (!active_)
  {
    return;
  }

  sub_scan_.reset();
  active_ = false;

  qInfo() << "[LaserScanSubscriber::stop] Subscriptions stopped";
}

bool LaserScanSubscriber::isActive() const { return active_; }

void LaserScanSubscriber::callbackScan(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
  if (!scan_data_)
  {
    qCritical() << "[LaserScanSubscriber::callbackScan] LaserScanData is null";
    return;
  }

  scan_data_->updateFromLaserScan(msg);
}
