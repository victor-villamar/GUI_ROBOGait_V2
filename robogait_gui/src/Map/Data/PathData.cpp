#include <iostream>

#include "Map/Data/PathData.hpp"

using namespace ROBOGait::map::data;

PathData::PathData() : is_available_(false), update_stamp_(0) { std::cout << "[PathData::PathData] Path data handler initialized" << std::endl; }

void PathData::setPath(const PathMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = metadata;
  is_available_ = true;
  ++update_stamp_;
}

PathData::PathMetadata PathData::getMetadata() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_;
}

std::vector<PathData::PathPoint> PathData::getPoints() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_.points;
}

bool PathData::isAvailable() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return is_available_;
}

void PathData::reset()
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = PathMetadata();
  is_available_ = false;
  ++update_stamp_;
}

uint64_t PathData::getUpdateStamp() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return update_stamp_;
}
