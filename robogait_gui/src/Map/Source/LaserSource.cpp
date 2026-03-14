#include <iostream>

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

void LaserSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    std::cerr << "[LaserSource::initialize] Null parent node pointer" << std::endl;
    return;
  }

  parent_node_ = parent_node;

  if (subscriber_)
  {
    subscriber_->initialize(parent_node_);
    subscriber_->setLaserScanData(scan_data_.get());

    if (context_)
    {
      subscriber_->setRobotContext(context_.value());
    }
  }

  initialized_ = true;
}

void LaserSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  if (subscriber_)
  {
    subscriber_->setRobotContext(context_.value());
  }
}

void LaserSource::start()
{
  if (!initialized_ || active_ || !subscriber_)
  {
    return;
  }

  subscriber_->start();

  active_ = true;
}

void LaserSource::stop()
{
  if (!active_ || !subscriber_)
  {
    return;
  }

  subscriber_->stop();

  if (scan_data_)
  {
    scan_data_->reset();
  }

  active_ = false;
}

bool LaserSource::isActive() const { return active_; }

bool LaserSource::isAvailable() const { return scan_data_ && scan_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::LaserScanData> LaserSource::getLaserScanData() const { return scan_data_; }
