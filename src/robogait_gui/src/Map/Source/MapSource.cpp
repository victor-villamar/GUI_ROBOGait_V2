#include <iostream>

#include "Map/Source/MapSource.hpp"

using namespace ROBOGait::map::source;

MapSource::MapSource() : map_data_(std::make_shared<ROBOGait::map::data::MapData>()), subscriber_(std::make_shared<ROBOGait::map::subscribers::MapSubscriber>())
{
  parent_node_ = nullptr;
  context_ = std::nullopt;
  initialized_ = false;
  active_ = false;
}

void MapSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    std::cerr << "[MapSource::initialize] Null parent node pointer" << std::endl;
    return;
  }

  parent_node_ = parent_node;

  if (subscriber_)
  {
    subscriber_->initialize(parent_node_);
    subscriber_->setMapData(map_data_.get());
    if (context_)
    {
      subscriber_->setRobotContext(context_.value());
    }
  }

  initialized_ = true;
}

void MapSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  if (subscriber_)
  {
    subscriber_->setRobotContext(context_.value());
  }
}

void MapSource::start()
{
  if (!initialized_ || active_)
  {
    return;
  }

  if (subscriber_)
  {
    subscriber_->start();
    active_ = subscriber_->isActive();
  }
}

void MapSource::stop()
{
  if (!active_)
  {
    return;
  }

  if (subscriber_)
  {
    subscriber_->stop();
  }

  if (map_data_)
  {
    map_data_->reset();
  }

  active_ = false;
}

bool MapSource::isActive() const
{
  if (subscriber_)
  {
    return subscriber_->isActive();
  }

  return active_;
}

bool MapSource::isAvailable() const { return map_data_ && map_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::MapData> MapSource::getMapData() const { return map_data_; }
