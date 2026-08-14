#include <algorithm>
#include <cstddef>
#include <utility>

#include "Map/Data/MapData.hpp"

using namespace ROBOGait::map::data;

MapData::MapDataResult::MapDataResult(bool success_in) : success(success_in) {}

MapData::MapDataResult::MapDataResult(const char* error_in) : success(false), error(error_in) {}

MapData::MapDataResult::MapDataResult(std::string error_in) : success(false), error(std::move(error_in)) {}

MapData::MapDataResult::operator bool() const { return success; }

MapData::MapData() : is_available_(false), update_stamp_(0) {}

MapData::MapMetadata MapData::getMetadata() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return metadata_;
}

MapData::MapDataResult MapData::setOccupancyData(const std::vector<int8_t>& occupancy_data, const MapMetadata& metadata)
{
  std::lock_guard<std::mutex> lock(data_mutex_);

  // Validate dimensions
  const size_t expected_size = metadata.width_ * metadata.height_;
  if (occupancy_data.size() != expected_size)
  {
    is_available_ = false;
    return MapDataResult("[MapData::setOccupancyData] Data size mismatch. Expected " + std::to_string(expected_size) + ", got " +
                         std::to_string(occupancy_data.size()));
  }

  if (metadata.width_ == 0 || metadata.height_ == 0)
  {
    is_available_ = false;
    return MapDataResult("[MapData::setOccupancyData] Invalid map dimensions: width=" + std::to_string(metadata.width_) +
                         ", height=" + std::to_string(metadata.height_));
  }

  metadata_ = metadata;

  if (occupancy_data_.size() != expected_size)
  {
    occupancy_data_.resize(expected_size);
  }

  std::copy(occupancy_data.begin(), occupancy_data.end(), occupancy_data_.begin());

  is_available_ = true;
  ++update_stamp_;
  return MapDataResult(true);
}

const std::vector<int8_t>& MapData::getOccupancyData() const
{
  std::lock_guard<std::mutex> lock(data_mutex_);
  return occupancy_data_;
}

MapData::MapDataResult MapData::updateRegion(int32_t x, int32_t y, uint32_t width, uint32_t height, const std::vector<int8_t>& data)
{
  std::lock_guard<std::mutex> lock(data_mutex_);

  if (!is_available_)
  {
    return MapDataResult("[MapData::updateRegion] Map data not available, cannot update region");
  }

  const size_t expected_size = static_cast<size_t>(width) * static_cast<size_t>(height);

  if (data.size() != expected_size)
  {
    return MapDataResult("[MapData::updateRegion] Data size mismatch. Expected " + std::to_string(expected_size) + ", got " + std::to_string(data.size()));
  }

  if (x < 0 || y < 0)
  {
    return MapDataResult("[MapData::updateRegion] Update region out of bounds");
  }

  const uint32_t update_x = static_cast<uint32_t>(x);
  const uint32_t update_y_start = static_cast<uint32_t>(y);

  if (update_x + width > metadata_.width_ || update_y_start + height > metadata_.height_)
  {
    return MapDataResult("[MapData::updateRegion] Update region out of bounds");
  }

  if (update_x == 0 && update_y_start == 0 && width == metadata_.width_ && height == metadata_.height_)
  {
    // Full map update, can replace data directly
    std::copy(data.begin(), data.end(), occupancy_data_.begin());
    ++update_stamp_;
    return MapDataResult(true);
  }

  if (update_x == 0 && width == metadata_.width_)
  {
    // Update the entire column
    const uint32_t start_index = update_y_start * metadata_.width_;
    std::copy(data.begin(), data.end(), occupancy_data_.begin() + start_index);
    ++update_stamp_;
    return MapDataResult(true);
  }

  for (uint32_t update_y = 0; update_y < height; ++update_y)
  {
    const uint32_t map_row_start = (update_y_start + update_y) * metadata_.width_ + update_x;
    const uint32_t update_row_start = update_y * width;

    // clang-format off
    std::copy(data.begin() + update_row_start,
              data.begin() + update_row_start + width,
              occupancy_data_.begin() + map_row_start);
    // clang-format on
  }

  ++update_stamp_;
  return MapDataResult(true);
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