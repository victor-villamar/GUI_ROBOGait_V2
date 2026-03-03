#include <algorithm>
#include <cmath>

#include <QDebug>

#include "Map/Data/MapData.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::data;

MapData::MapData() : image_dirty_(false), is_available_(false), update_stamp_(0), has_context_(false)
{
  qInfo() << "[MapData::MapData] Map data handler initialized";
}

void MapData::updateFromOccupancyGrid(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  QMutexLocker lock(&data_mutex_);

  if (!msg)
  {
    qCritical() << "[MapData::updateFromOccupancyGrid] Received null message pointer";
    return;
  }

  // Extract metadata
  metadata_.resolution = msg->info.resolution;
  metadata_.width = msg->info.width;
  metadata_.height = msg->info.height;
  metadata_.origin_x = msg->info.origin.position.x;
  metadata_.origin_y = msg->info.origin.position.y;

  // Extract origin orientation (quaternion to yaw angle)
  metadata_.origin_theta = utils::getYaw(msg->info.origin.orientation);

  // Validate occupancy data dimensions
  const size_t expected_size = static_cast<size_t>(metadata_.width) * static_cast<size_t>(metadata_.height);

  if (msg->data.size() != expected_size)
  {
    qCritical() << "[MapData::updateFromOccupancyGrid] Data size mismatch! Expected:" << expected_size << "Got:" << msg->data.size();
    is_available_ = false;
    image_dirty_ = false;
    return;
  }

  // Copy occupancy data
  if (occupancy_data_.size() != expected_size)
  {
    occupancy_data_.resize(expected_size);
  }

  std::copy(msg->data.begin(), msg->data.end(), occupancy_data_.begin());

  image_dirty_ = true;
  is_available_ = true;
  ++update_stamp_;
}

void MapData::updateFromOccupancyGridUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg)
{
  QMutexLocker lock(&data_mutex_);

  if (!msg)
  {
    qCritical() << "[MapData::updateFromOccupancyGridUpdate] Received null message pointer";
    return;
  }

  if (!is_available_)
  {
    qWarning() << "[MapData::updateFromOccupancyGridUpdate] Cannot apply update: base map not initialized yet";
    return;
  }

  // Validate update dimensions
  const size_t expected_size = static_cast<size_t>(msg->width) * static_cast<size_t>(msg->height);
  if (msg->data.size() != expected_size)
  {
    qCritical() << "[MapData::updateFromOccupancyGridUpdate] Update data size mismatch! Expected:" << expected_size << "Got:" << msg->data.size();
    return;
  }

  // Validate update region is within map bounds
  if (msg->x + msg->width > metadata_.width || msg->y + msg->height > metadata_.height)
  {
    qCritical() << "[MapData::updateFromOccupancyGridUpdate] Update region out of bounds!"
                << "Update region: [" << msg->x << "," << msg->y << "] size [" << msg->width << "x" << msg->height << "]"
                << "Map size: [" << metadata_.width << "x" << metadata_.height << "]";
    return;
  }

  // Handle full map updates
  if (msg->x == 0 && msg->y == 0 && msg->width == metadata_.width && msg->height == metadata_.height)
  {
    std::copy(msg->data.begin(), msg->data.end(), occupancy_data_.begin());
    image_dirty_ = true;
    ++update_stamp_;
    return;
  }

  // Handle full-width updates
  if (msg->x == 0 && msg->width == metadata_.width)
  {
    const uint32_t map_start = msg->y * metadata_.width;

    std::copy(msg->data.begin(), msg->data.end(), occupancy_data_.begin() + map_start);

    image_dirty_ = true;
    ++update_stamp_;
    return;
  }

  // Apply incremental update to occupancy data
  for (uint32_t update_y = 0; update_y < msg->height; ++update_y)
  {
    const uint32_t map_row_start = (msg->y + update_y) * metadata_.width + msg->x;
    const uint32_t update_row_start = update_y * msg->width;

    std::copy(msg->data.begin() + update_row_start, msg->data.begin() + update_row_start + msg->width, occupancy_data_.begin() + map_row_start);
  }

  image_dirty_ = true;
  ++update_stamp_;

  // DEBUG
  // qDebug() << "[MapData::updateFromOccupancyGridUpdate] Applied incremental update:"
  //          << "region [" << msg->x << "," << msg->y << "] size [" << msg->width << "x" << msg->height << "]";
}

QImage MapData::toQImage()
{
  QMutexLocker lock(&data_mutex_);

  if (!is_available_)
  {
    qWarning() << "[MapData::toQImage] Map data not available yet";
    return QImage();
  }

  if (image_dirty_)
  {
    regenerateImage();
    image_dirty_ = false;
  }

  return cached_image_;
}

void MapData::regenerateImage()
{
  uint32_t width = metadata_.width;
  uint32_t height = metadata_.height;

  cached_image_ = QImage(width, height, QImage::Format_RGB888);

  if (cached_image_.isNull())
  {
    qCritical() << "[MapData::regenerateImage] Failed to create QImage";
    return;
  }

  for (uint32_t y = 0; y < height; ++y)
  {
    for (uint32_t x = 0; x < width; ++x)
    {
      const uint32_t index = y * width + x;
      const int8_t occupancy = occupancy_data_[index];

      QRgb color;

      if (occupancy == -1)
      {
        // Unknown: gray
        color = qRgb(26, 58, 74);
      }
      else if (occupancy < 50)
      {
        // Free space: white
        color = qRgb(169, 207, 232);
      }
      else
      {
        // Occupied: black
        color = qRgb(255, 255, 255);
      }

      const uint32_t flipped_y = height - 1 - y;
      cached_image_.setPixel(x, flipped_y, color);
    }
  }
}

MapData::MapMetadata MapData::getMetadata() const
{
  QMutexLocker lock(&data_mutex_);
  return metadata_;
}

bool MapData::isAvailable() const { return is_available_; }

uint64_t MapData::getUpdateStamp() const
{
  QMutexLocker lock(&data_mutex_);
  return update_stamp_;
}

void MapData::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  QMutexLocker lock(&data_mutex_);
  context_ = context;
  has_context_ = true;
}

bool MapData::hasRobotContext() const { return has_context_; }

std::string MapData::mapTopic() const
{
  QMutexLocker lock(&data_mutex_);
  const std::string base = std::string(T_MAP);
  if (!has_context_)
  {
    return base;
  }

  return context_.resolveTopic(base);
}

std::string MapData::mapUpdatesTopic() const
{
  QMutexLocker lock(&data_mutex_);
  const std::string base = std::string(T_MAP_UPDATES);
  if (!has_context_)
  {
    return base;
  }

  return context_.resolveTopic(base);
}
