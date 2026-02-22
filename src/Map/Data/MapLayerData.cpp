#include <QDebug>
#include <algorithm>
#include <cmath>

#include "Map/Data/MapLayerData.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::map::data;

MapLayerData::MapLayerData() : image_dirty_(false), is_available_(false), has_context_(false)
{
  qInfo() << "[MapLayerData::MapLayerData] Map data handler initialized";
}

void MapLayerData::updateFromOccupancyGrid(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  if (!msg)
  {
    qCritical() << "[MapLayerData::updateFromOccupancyGrid] Received null message pointer";
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
    qCritical() << "[MapLayerData::updateFromOccupancyGrid] Data size mismatch! Expected:" << expected_size << "Got:" << msg->data.size();
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
}

void MapLayerData::updateFromOccupancyGridUpdate(const map_msgs::msg::OccupancyGridUpdate::SharedPtr msg)
{
  if (!msg)
  {
    qCritical() << "[MapLayerData::updateFromOccupancyGridUpdate] Received null message pointer";
    return;
  }

  if (!is_available_)
  {
    qWarning() << "[MapLayerData::updateFromOccupancyGridUpdate] Cannot apply update: base map not initialized yet";
    return;
  }

  // Validate update dimensions
  const size_t expected_size = static_cast<size_t>(msg->width) * static_cast<size_t>(msg->height);
  if (msg->data.size() != expected_size)
  {
    qCritical() << "[MapLayerData::updateFromOccupancyGridUpdate] Update data size mismatch! Expected:" << expected_size << "Got:" << msg->data.size();
    return;
  }

  // Validate update region is within map bounds
  if (msg->x + msg->width > metadata_.width || msg->y + msg->height > metadata_.height)
  {
    qCritical() << "[MapLayerData::updateFromOccupancyGridUpdate] Update region out of bounds!"
                << "Update region: [" << msg->x << "," << msg->y << "] size [" << msg->width << "x" << msg->height << "]"
                << "Map size: [" << metadata_.width << "x" << metadata_.height << "]";
    return;
  }

  // Handle full map updates
  if (msg->x == 0 && msg->y == 0 && msg->width == metadata_.width && msg->height == metadata_.height)
  {
    std::copy(msg->data.begin(), msg->data.end(), occupancy_data_.begin());
    image_dirty_ = true;
    return;
  }

  // Handle full-width updates
  if (msg->x == 0 && msg->width == metadata_.width)
  {
    const uint32_t map_start = msg->y * metadata_.width;

    std::copy(msg->data.begin(), msg->data.end(), occupancy_data_.begin() + map_start);

    image_dirty_ = true;
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

  // DEBUG
  // qDebug() << "[MapLayerData::updateFromOccupancyGridUpdate] Applied incremental update:"
  //          << "region [" << msg->x << "," << msg->y << "] size [" << msg->width << "x" << msg->height << "]";
}

QImage MapLayerData::toQImage()
{
  if (!is_available_)
  {
    qWarning() << "[MapLayerData::toQImage] Map data not available yet";
    return QImage();
  }

  if (image_dirty_)
  {
    regenerateImage();
    image_dirty_ = false;
  }

  return cached_image_;
}

void MapLayerData::regenerateImage()
{
  uint32_t width = metadata_.width;
  uint32_t height = metadata_.height;

  cached_image_ = QImage(width, height, QImage::Format_RGB888);

  if (cached_image_.isNull())
  {
    qCritical() << "[MapLayerData::regenerateImage] Failed to create QImage";
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
        color = qRgb(128, 128, 128);
      }
      else if (occupancy < 50)
      {
        // Free space: white
        color = qRgb(255, 255, 255);
      }
      else
      {
        // Occupied: black
        color = qRgb(0, 0, 0);
      }

      const uint32_t flipped_y = height - 1 - y;
      cached_image_.setPixel(x, flipped_y, color);
    }
  }
}

const MapMetadata& MapLayerData::getMetadata() const { return metadata_; }

bool MapLayerData::isAvailable() const { return is_available_; }

void MapLayerData::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;
  has_context_ = true;
}

bool MapLayerData::hasRobotContext() const { return has_context_; }

std::string MapLayerData::mapTopic() const
{
  const std::string base = std::string(T_MAP);
  if (!has_context_)
  {
    return base;
  }

  return context_.resolveTopic(base);
}

std::string MapLayerData::mapUpdatesTopic() const
{
  const std::string base = std::string(T_MAP_UPDATES);
  if (!has_context_)
  {
    return base;
  }

  return context_.resolveTopic(base);
}
