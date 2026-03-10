#include <QDebug>

#include "Map/Source/LaserSource.hpp"

using namespace ROBOGait::map::source;

LaserSource::LaserSource() :
    scan_data_(std::make_shared<ROBOGait::map::data::LaserScanData>()), subscriber_(std::make_shared<ROBOGait::map::data::LaserScanSubscriber>())
{
  parent_node_ = nullptr;
  context_ = ROBOGait::context::RobotContext();
  has_context_ = false;
  initialized_ = false;
  active_ = false;
}

void LaserSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[LaserSource::initialize] Null parent node pointer";
    return;
  }

  parent_node_ = parent_node;

  if (subscriber_)
  {
    subscriber_->initialize(parent_node_);

    if (scan_data_)
    {
      scan_data_->initialize(parent_node_);
    }

    subscriber_->setScanData(scan_data_);

    if (has_context_)
    {
      subscriber_->setRobotContext(context_);
    }
  }

  initialized_ = true;
}

void LaserSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;
  if (subscriber_)
  {
    subscriber_->setRobotContext(context_);
  }
}

void LaserSource::start()
{
  if (!initialized_ || active_)
  {
    return;
  }

  if (scan_data_)
  {
    scan_data_->startTFListener();
  }

  if (subscriber_)
  {
    subscriber_->start();
    active_ = subscriber_->isActive();
  }
}

void LaserSource::stop()
{
  if (!active_)
  {
    return;
  }

  if (subscriber_)
  {
    subscriber_->stop();
  }

  if (scan_data_)
  {
    scan_data_->stopTFListener();
    scan_data_->reset();
  }

  active_ = false;
}

bool LaserSource::isActive() const
{
  if (subscriber_)
  {
    return subscriber_->isActive();
  }

  return active_;
}

bool LaserSource::isAvailable() const { return scan_data_ && scan_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::LaserScanData> LaserSource::getLaserScanData() const { return scan_data_; }
