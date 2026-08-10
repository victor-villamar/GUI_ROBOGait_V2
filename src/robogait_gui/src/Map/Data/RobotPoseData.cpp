#include <iostream>

#include "Map/Data/RobotPoseData.hpp"

using namespace ROBOGait::map::data;

RobotPoseData::RobotPoseData() : is_available_(false), update_stamp_(0)
{
  std::cout << "[RobotPoseData::RobotPoseData] Robot pose data handler initialized" << std::endl;
}

void RobotPoseData::setPose(const RobotPoseData::RobotPoseMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = metadata;
  is_available_ = true;
  ++update_stamp_;
}

RobotPoseData::RobotPoseMetadata RobotPoseData::getMetadata() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_;
}

bool RobotPoseData::isAvailable() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return is_available_;
}

void RobotPoseData::reset()
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  metadata_ = RobotPoseMetadata();
  is_available_ = false;
  ++update_stamp_;
}

uint64_t RobotPoseData::getUpdateStamp() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return update_stamp_;
}
