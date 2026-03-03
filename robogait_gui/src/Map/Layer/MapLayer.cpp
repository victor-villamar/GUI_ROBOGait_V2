#include <QDebug>

#include "Map/Layer/MapLayer.hpp"

using namespace ROBOGait::map::layer;

MapLayer::MapLayer(std::shared_ptr<data::MapData> map_data) : map_data_(std::move(map_data)), last_stamp_(0), render_requested_(false) {}

std::shared_ptr<ROBOGait::map::data::MapData> MapLayer::getMapData() const { return map_data_; }

const QImage& MapLayer::getImage() const
{
  QMutexLocker lock(&image_mutex_);
  return cached_image_;
}

void MapLayer::update()
{
  if (!map_data_ || !map_data_->isAvailable())
  {
    return;
  }

  const uint64_t stamp = map_data_->getUpdateStamp();

  if (stamp != last_stamp_)
  {
    refreshImage();
    last_stamp_ = stamp;
    render_requested_ = true;
  }
}

bool MapLayer::needsRender() const { return render_requested_; }

void MapLayer::clearRenderRequest() { render_requested_ = false; }

void MapLayer::refreshImage()
{
  if (!map_data_)
  {
    return;
  }

  QImage image = map_data_->toQImage();

  if (image.isNull())
  {
    qWarning() << "[MapLayer::refreshImage] Map image is null";
    return;
  }

  QMutexLocker lock(&image_mutex_);
  cached_image_ = std::move(image);
}
