#include <iostream>

#include "Map/Data/ParticleCloudData.hpp"

using namespace ROBOGait::map::data;

ParticleCloudData::ParticleCloudData() : is_available_(false), update_stamp_(0)
{
  std::cout << "[ParticleCloudData::ParticleCloudData] Particle cloud data handler initialized" << std::endl;
}

void ParticleCloudData::setParticleCloudData(const ParticleCloudMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = metadata;
  is_available_ = true;
  ++update_stamp_;
}

ParticleCloudData::ParticleCloudMetadata ParticleCloudData::getMetadata() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_;
}

std::vector<ParticleCloudData::Particle> ParticleCloudData::getParticles() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_.particles;
}

bool ParticleCloudData::isAvailable() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return is_available_;
}

void ParticleCloudData::reset()
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = ParticleCloudMetadata();
  is_available_ = false;
  ++update_stamp_;
}

uint64_t ParticleCloudData::getUpdateStamp() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return update_stamp_;
}
