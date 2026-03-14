#include "Map/Layer/LaserLayer.hpp"

using namespace ROBOGait::map::layer;

LaserLayer::LaserLayer() : last_stamp_(0), render_requested_(false) {}

void LaserLayer::setLaserScanData(std::shared_ptr<data::LaserScanData> scan_data)
{
  QMutexLocker lock(&data_mutex_);
  scan_data_ = std::move(scan_data);
}
void LaserLayer::update()
{
  if (!scan_data_)
  {
    return;
  }

  const uint64_t stamp = scan_data_->getUpdateStamp();

  if (stamp == last_stamp_)
  {
    return;
  }

  std::vector<QPointF> points;
  const bool available = scan_data_->getPoints(points);

  QMutexLocker lock(&data_mutex_);
  if (!available)
  {
    if (!cached_points_.empty())
    {
      cached_points_.clear();
      render_requested_ = true;
    }
    last_stamp_ = stamp;
    return;
  }

  cached_points_ = std::move(points);
  render_requested_ = true;
  last_stamp_ = stamp;
}

bool LaserLayer::needsRender() const { return render_requested_; }

void LaserLayer::clearRenderRequest() { render_requested_ = false; }

std::vector<QPointF> LaserLayer::getPoints() const
{
  QMutexLocker lock(&data_mutex_);
  return cached_points_;
}
