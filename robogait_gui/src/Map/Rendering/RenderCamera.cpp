#include <QtMath>

#include "Map/Rendering/RenderCamera.hpp"

using namespace ROBOGait::map::rendering;

RenderCamera::RenderCamera() : viewport_(QSizeF(1.0, 1.0)), center_(QPointF(0.0, 0.0)), zoom_(1.0) {}

void RenderCamera::setViewportSize(const QSizeF& size)
{
  QMutexLocker lock(&mutex_);
  viewport_ = size;
}

void RenderCamera::setViewCenter(const QPointF& center)
{
  QMutexLocker lock(&mutex_);
  center_ = center;
}

QPointF RenderCamera::getViewCenter() const
{
  QMutexLocker lock(&mutex_);
  return center_;
}

void RenderCamera::setZoom(double zoom)
{
  QMutexLocker lock(&mutex_);
  zoom_ = clampZoom(zoom);
}

double RenderCamera::getZoom() const
{
  QMutexLocker lock(&mutex_);
  return zoom_;
}

void RenderCamera::zoomByFactor(double factor)
{
  QMutexLocker lock(&mutex_);
  zoom_ = clampZoom(zoom_ * factor);
}

void RenderCamera::fitToRect(const QRectF& rect, double margin)
{
  if (rect.isEmpty())
  {
    qCritical() << "[RenderCamera::fitToRect] Cannot fit to an empty rectangle";
    return;
  }

  QMutexLocker lock(&mutex_);
  const double w = qMax(1.0, viewport_.width());
  const double h = qMax(1.0, viewport_.height());
  const double zoom_x = w / (rect.width() * margin);
  const double zoom_y = h / (rect.height() * margin);
  zoom_ = clampZoom(qMin(zoom_x, zoom_y));
  center_ = rect.center();
}

QMatrix4x4 RenderCamera::getMatrix() const
{
  QMutexLocker lock(&mutex_);
  QMatrix4x4 matrix;
  matrix.translate(viewport_.width() * VIEWPORT_FACTOR, viewport_.height() * VIEWPORT_FACTOR);
  matrix.scale(zoom_, zoom_);
  matrix.translate(-center_.x(), -center_.y());
  return matrix;
}

double RenderCamera::clampZoom(double zoom)
{
  if (zoom < MIN_ZOOM)
  {
    return MIN_ZOOM;
  }
  if (zoom > MAX_ZOOM)
  {
    return MAX_ZOOM;
  }
  return zoom;
}
