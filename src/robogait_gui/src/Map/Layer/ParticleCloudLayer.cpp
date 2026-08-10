#include "Map/Layer/ParticleCloudLayer.hpp"

using namespace ROBOGait::map::layer;

ParticleCloudLayer::ParticleCloudLayer() : last_stamp_(0), render_requested_(false) {}

void ParticleCloudLayer::setParticleCloudData(std::shared_ptr<data::ParticleCloudData> cloud_data)
{
  QMutexLocker lock(&data_mutex_);
  cloud_data_ = std::move(cloud_data);
}

void ParticleCloudLayer::update()
{
  if (!cloud_data_)
  {
    return;
  }

  const uint64_t stamp = cloud_data_->getUpdateStamp();

  if (stamp == last_stamp_)
  {
    return;
  }

  std::vector<data::ParticleCloudData::Particle> particles = cloud_data_->getParticles();

  QMutexLocker lock(&data_mutex_);
  if (!cloud_data_->isAvailable() || particles.empty())
  {
    if (!cached_particles_.empty())
    {
      cached_particles_.clear();
      render_requested_ = true;
    }
    last_stamp_ = stamp;
    return;
  }

  cached_particles_ = std::move(particles);
  render_requested_ = true;
  last_stamp_ = stamp;
}

bool ParticleCloudLayer::needsRender() const { return render_requested_; }

void ParticleCloudLayer::clearRenderRequest() { render_requested_ = false; }

std::vector<ROBOGait::map::data::ParticleCloudData::Particle> ParticleCloudLayer::getParticles() const
{
  QMutexLocker lock(&data_mutex_);
  return cached_particles_;
}
