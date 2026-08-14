#include "Map/Source/ParticleCloudSource.hpp"

using namespace ROBOGait::map::source;

ParticleCloudSource::ParticleCloudSource()
{
  parent_node_ = nullptr;
  context_ = std::nullopt;
  initialized_ = false;
  active_ = false;

  cloud_data_ = std::make_shared<ROBOGait::map::data::ParticleCloudData>();
  subscriber_ = std::make_shared<ROBOGait::map::subscribers::ParticleCloudSubscriber>();
}

SourceInterface::SourceResult ParticleCloudSource::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    return SourceResult::failure("[ParticleCloudSource::initialize] Null parent node pointer");
  }

  parent_node_ = parent_node;

  if (subscriber_)
  {
    subscriber_->initialize(parent_node_);
    subscriber_->setParticleCloudData(cloud_data_.get());

    if (context_)
    {
      subscriber_->setRobotContext(context_.value());
    }
  }

  initialized_ = true;
  return SourceResult::success();
}

void ParticleCloudSource::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  if (subscriber_)
  {
    subscriber_->setRobotContext(context_.value());
  }
}

void ParticleCloudSource::start()
{
  if (!initialized_ || active_ || !subscriber_)
  {
    return;
  }

  subscriber_->start();

  active_ = true;
}

void ParticleCloudSource::stop()
{
  if (!active_ || !subscriber_)
  {
    return;
  }

  subscriber_->stop();

  if (cloud_data_)
  {
    cloud_data_->reset();
  }

  active_ = false;
}

bool ParticleCloudSource::isActive() const { return active_; }

bool ParticleCloudSource::isAvailable() const { return cloud_data_ && cloud_data_->isAvailable(); }

std::shared_ptr<ROBOGait::map::data::ParticleCloudData> ParticleCloudSource::getParticleCloudData() const { return cloud_data_; }
