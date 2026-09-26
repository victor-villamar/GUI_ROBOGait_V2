#include "Map/Source/LaserSource.hpp"

using namespace ROBOGait::map::source;

LaserSource::LaserSource()
{
  parent_node_ = nullptr;
  context_ = std::nullopt;
  initialized_ = false;
  active_ = false;

  scan_data_ = std::make_shared<ROBOGait::map::data::LaserScanData>();
  subscriber_ = std::make_shared<ROBOGait::map::subscribers::LaserScanSubscriber>();
}

SourceInterface::SourceResult LaserSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    return SourceResult::failure("[LaserSource::initialize] Null parent node pointer");
  }

  parent_node_ = parent_node;

  if (subscriber_)
  {
    subscriber_->initialize(parent_node_);
    subscriber_->setLaserScanData(scan_data_.get());

    if (tf_buffer_)
    {
      subscriber_->setTFBuffer(tf_buffer_);
    }

    if (context_)
    {
      subscriber_->setRobotContext(context_.value());
    }
  }

  initialized_ = true;
  return SourceResult::success();
}

void LaserSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  if (subscriber_)
  {
    subscriber_->setRobotContext(context_.value());
  }
}

void LaserSource::setTFBuffer(const std::shared_ptr<tf2_ros::Buffer>& tf_buffer)
{
  tf_buffer_ = tf_buffer;

  if (subscriber_)
  {
    subscriber_->setTFBuffer(tf_buffer_);
  }
}

void LaserSource::start()
{
  if (!initialized_ || active_ || !subscriber_)
  {
    return;
  }

  subscriber_->start();
  active_ = subscriber_->isActive();
}

void LaserSource::stop()
{
  if (subscriber_)
  {
    subscriber_->stop();
  }

  tf_buffer_.reset();

  if (scan_data_)
  {
    scan_data_->reset();
  }

  active_ = false;
}

bool LaserSource::isActive() const { return active_; }

bool LaserSource::isAvailable() const { return scan_data_ && scan_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::LaserScanData> LaserSource::getLaserScanData() const { return scan_data_; }