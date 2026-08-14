#include "Map/Data/LaserScanData.hpp"

using namespace ROBOGait::map::data;

LaserScanData::LaserScanData() : is_available_(false), update_stamp_(0) {}

void LaserScanData::setLaserScanData(const LaserScanMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = metadata;
  is_available_ = true;
  ++update_stamp_;
}

LaserScanData::LaserScanMetadata LaserScanData::getMetadata() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_;
}

std::vector<LaserScanData::LaserPoint> LaserScanData::getPoints() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_.points;
}

bool LaserScanData::isAvailable() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return is_available_;
}

void LaserScanData::reset()
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = LaserScanMetadata();
  is_available_ = false;
  ++update_stamp_;
}

uint64_t LaserScanData::getUpdateStamp() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return update_stamp_;
}