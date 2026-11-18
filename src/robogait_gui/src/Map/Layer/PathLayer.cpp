#include "Map/Layer/PathLayer.hpp"

using namespace ROBOGait::map::layer;

PathLayer::PathLayer() : last_stamp_(0), render_requested_(false) {}

void PathLayer::setPathData(std::shared_ptr<data::PathData> path_data)
{
  QMutexLocker lock(&data_mutex_);
  path_data_ = std::move(path_data);
}

void PathLayer::update()
{
  if (!path_data_)
  {
    return;
  }

  const uint64_t stamp = path_data_->getUpdateStamp();

  if (stamp == last_stamp_)
  {
    return;
  }

  std::vector<data::PathData::PathPoint> points = path_data_->getPoints();

  QMutexLocker lock(&data_mutex_);
  if (!path_data_->isAvailable() || points.empty())
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

bool PathLayer::needsRender() const { return render_requested_; }

void PathLayer::clearRenderRequest() { render_requested_ = false; }

std::vector<ROBOGait::map::data::PathData::PathPoint> PathLayer::getPoints() const
{
  QMutexLocker lock(&data_mutex_);
  return cached_points_;
}
