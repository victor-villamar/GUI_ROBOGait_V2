#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>

#include "Map/Data/MapData.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::data;

MapData::MapData() : is_available_(false), update_stamp_(0) { std::cout << "[MapData::MapData] Map data handler initialized" << std::endl; }

MapData::MapMetadata MapData::getMetadata() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_;
}

void MapData::setOccupancyData(const std::vector<int8_t>& occupancy_data, const MapMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(data_mutex_);

  // Validate dimensions
  const size_t expected_size = metadata.width * metadata.height;
  if (occupancy_data.size() != expected_size)
  {
    std::cerr << "[MapData::setOccupancyData] Data size mismatch! Expected " << expected_size << ", got " << occupancy_data.size() << std::endl;
    is_available_ = false;
    return;
  }

  if (metadata.width == 0 || metadata.height == 0)
  {
    std::cerr << "[MapData::setOccupancyData] Invalid map dimensions: width=" << metadata.width << ", height=" << metadata.height << std::endl;
    is_available_ = false;
    return;
  }

  metadata_ = metadata;

  if (occupancy_data_.size() != expected_size)
  {
    occupancy_data_.resize(expected_size);
  }

  std::copy(occupancy_data.begin(), occupancy_data.end(), occupancy_data_.begin());

  is_available_ = true;
  ++update_stamp_;
}

const std::vector<int8_t>& MapData::getOccupancyData() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return occupancy_data_;
}

void MapData::updateRegion(int32_t x, int32_t y, uint32_t width, uint32_t height, const std::vector<int8_t>& data)
{
  std::lock_guard<std::mutex> lock(data_mutex_);

  if (!is_available_)
  {
    std::cerr << "[MapData::updateRegion] Map data not available, cannot update region" << std::endl;
    return;
  }

  const size_t expected_size = static_cast<size_t>(width) * static_cast<size_t>(height);
  if (data.size() != expected_size)
  {
    std::cerr << "[MapData::updateRegion] Data size mismatch! Expected " << expected_size << ", got " << data.size() << std::endl;
    return;
  }

  if (x + width > metadata_.width || y + height > metadata_.height)
  {
    std::cerr << "[MapData::updateRegion] Update region out of bounds" << std::endl;
    return;
  }

  if (x == 0 && y == 0 && width == metadata_.width && height == metadata_.height)
  {
    // Full map update, can replace data directly
    std::copy(data.begin(), data.end(), occupancy_data_.begin());
    ++update_stamp_;
    return;
  }

  if (x == 0 && width == metadata_.width)
  {
    // Update the entire column
    const uint32_t start_index = y * metadata_.width;
    std::copy(data.begin(), data.end(), occupancy_data_.begin() + start_index);
    ++update_stamp_;
    return;
  }

  for (uint32_t update_y = 0; update_y < height; ++update_y)
  {
    const uint32_t map_row_start = (y + update_y) * metadata_.width + x;
    const uint32_t update_row_start = update_y * width;

    // clang-format off
    std::copy(data.begin() + update_row_start,
              data.begin() + update_row_start + width,
              occupancy_data_.begin() + map_row_start);
    // clang-format on
  }

  ++update_stamp_;
}

bool MapData::isAvailable() const { return is_available_; }

void MapData::reset()
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  occupancy_data_.clear();
  metadata_ = MapMetadata{};
  is_available_ = false;
  ++update_stamp_;
}

uint64_t MapData::getUpdateStamp() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return update_stamp_;
}