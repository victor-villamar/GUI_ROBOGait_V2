#include "Map/Source/PathSource.hpp"

using namespace ROBOGait::map::source;

PathSource::PathSource()
{
  parent_node_ = nullptr;
  context_ = std::nullopt;
  initialized_ = false;
  active_ = false;

  path_data_ = std::make_shared<ROBOGait::map::data::PathData>();
  subscriber_ = std::make_shared<ROBOGait::map::subscribers::PathSubscriber>();
}

SourceInterface::SourceResult PathSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    return SourceResult::failure("[PathSource::initialize] Null parent node pointer");
  }

  parent_node_ = parent_node;

  if (subscriber_)
  {
    subscriber_->initialize(parent_node_);
    subscriber_->setPathData(path_data_.get());

    if (context_)
    {
      subscriber_->setRobotContext(context_.value());
    }
  }

  initialized_ = true;
  return SourceResult::success();
}

void PathSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  if (subscriber_)
  {
    subscriber_->setRobotContext(context_.value());
  }
}

void PathSource::start()
{
  if (!initialized_ || active_ || !subscriber_)
  {
    return;
  }

  subscriber_->start();

  active_ = true;
}

void PathSource::stop()
{
  if (!active_ || !subscriber_)
  {
    return;
  }

  subscriber_->stop();

  if (path_data_)
  {
    path_data_->reset();
  }

  active_ = false;
}

bool PathSource::isActive() const { return active_; }

bool PathSource::isAvailable() const { return path_data_ && path_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::PathData> PathSource::getPathData() const { return path_data_; }
