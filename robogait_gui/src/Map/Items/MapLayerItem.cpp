#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QGestureEvent>
#include <QLineF>
#include <QPinchGesture>
#include <QSGTexture>
#include <QSGTransformNode>
#include <QtGlobal>
#include <QtMath>

#include "Map/Items/MapLayerItem.hpp"

using namespace ROBOGait::map::item;

MapLayerItem::MapLayerItem(QQuickItem* parent) :
    QQuickItem(parent),
    last_image_key_(0),
    map_rect_(),
    has_map_rect_(false),
    fit_done_(false),
    is_panning_(false),
    panning_enabled_(true),
    is_pinching_(false),
    pinch_start_zoom_(1.0),
    pinch_start_distance_(0.0)
{
  setFlag(ItemHasContents, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);
  setAcceptTouchEvents(true);

  qInfo() << "[MapLayerItem::MapLayerItem] MapLayerItem created";
}

MapLayerItem::~MapLayerItem() { qInfo() << "[MapLayerItem::~MapLayerItem] MapLayerItem destroyed"; }

void MapLayerItem::setRenderScene(const std::shared_ptr<ROBOGait::map::rendering::RenderScene>& scene)
{
  if (pipeline_)
  {
    disconnect(pipeline_, nullptr, this, nullptr);
  }

  render_scene_ = scene;
  pipeline_ = nullptr;

  if (render_scene_ && render_scene_->getPipeline())
  {
    pipeline_ = render_scene_->getPipeline().get();

    // clang-format off
    connect(pipeline_,
            &ROBOGait::map::rendering::RenderPipeline::frameReady,
            this,
            &MapLayerItem::onFrameReady);
    // clang-format on
  }
}

void MapLayerItem::setRenderer(const std::shared_ptr<ROBOGait::map::layer::MapLayer>& map_render)
{
  map_render_ = map_render;
  update();
}

void MapLayerItem::setCamera(const std::shared_ptr<ROBOGait::map::rendering::RenderCamera>& camera)
{
  camera_ = camera;
  fit_done_ = false;
  update();
}

void MapLayerItem::setSyncItem(QQuickItem* item)
{
  if (!item)
  {
    return;
  }

  for (const auto& sync_item : sync_items_)
  {
    if (sync_item == item)
    {
      return;
    }
  }

  sync_items_.append(item);
}

bool MapLayerItem::isPanningEnabled() const { return panning_enabled_; }

void MapLayerItem::setPanningEnabled(bool enabled)
{
  if (panning_enabled_ == enabled)
  {
    return;
  }
  panning_enabled_ = enabled;
  if (!panning_enabled_)
  {
    is_panning_ = false;
  }
  emit panningEnabledChanged();
}

QSGNode* MapLayerItem::updatePaintNode(QSGNode* old_node, UpdatePaintNodeData* data)
{
  Q_UNUSED(data);

  if (!map_render_ || !window())
  {
    has_map_rect_ = false;
    delete old_node;
    return nullptr;
  }

  auto* transform_node = static_cast<QSGTransformNode*>(old_node);
  if (!transform_node)
  {
    transform_node = new QSGTransformNode();
  }

  const QImage image = map_render_->getImage();
  if (image.isNull())
  {
    has_map_rect_ = false;
    delete transform_node;
    return nullptr;
  }

  auto* map_transform_node = dynamic_cast<QSGTransformNode*>(transform_node->firstChild());
  if (!map_transform_node)
  {
    delete transform_node;
    transform_node = new QSGTransformNode();
    map_transform_node = new QSGTransformNode();
    transform_node->appendChildNode(map_transform_node);
  }

  QSGSimpleTextureNode* node = nullptr;
  if (!map_transform_node->firstChild())
  {
    node = new QSGSimpleTextureNode();
    map_transform_node->appendChildNode(node);
  }
  else
  {
    node = static_cast<QSGSimpleTextureNode*>(map_transform_node->firstChild());
  }

  const qint64 key = image.cacheKey();
  if (key != last_image_key_ || !node->texture())
  {
    QSGTexture* texture = window()->createTextureFromImage(image);
    texture->setFiltering(QSGTexture::Nearest);
    texture->setHorizontalWrapMode(QSGTexture::ClampToEdge);
    texture->setVerticalWrapMode(QSGTexture::ClampToEdge);
    node->setTexture(texture);
    node->setOwnsTexture(true);
    last_image_key_ = key;
  }

  QRectF map_rect(0, 0, image.width(), image.height());
  QMatrix4x4 map_matrix;
  map_matrix.setToIdentity();

  if (map_render_->getMapData())
  {
    auto metadata = map_render_->getMapData()->getMetadata();
    const double width_m = static_cast<double>(metadata.width) * metadata.resolution;
    const double height_m = static_cast<double>(metadata.height) * metadata.resolution;

    if (width_m > 0.0 && height_m > 0.0)
    {
      node->setRect(0.0, 0.0, width_m, height_m);

      const double origin_x = metadata.origin_x;
      const double origin_y = metadata.origin_y;
      const double theta = metadata.origin_theta;
      const double cos_t = std::cos(theta);
      const double sin_t = std::sin(theta);

      auto mapPoint = [&](double x, double y) { return QPointF(origin_x + cos_t * x - sin_t * y, origin_y + sin_t * x + cos_t * y); };

      const QPointF p0 = mapPoint(0.0, 0.0);
      const QPointF p1 = mapPoint(width_m, 0.0);
      const QPointF p2 = mapPoint(0.0, height_m);
      const QPointF p3 = mapPoint(width_m, height_m);

      const double min_x = std::min({p0.x(), p1.x(), p2.x(), p3.x()});
      const double max_x = std::max({p0.x(), p1.x(), p2.x(), p3.x()});
      const double min_y = std::min({p0.y(), p1.y(), p2.y(), p3.y()});
      const double max_y = std::max({p0.y(), p1.y(), p2.y(), p3.y()});

      map_rect = QRectF(QPointF(min_x, min_y), QPointF(max_x, max_y));

      map_matrix.translate(origin_x, origin_y);
      map_matrix.rotate(qRadiansToDegrees(theta), 0.0f, 0.0f, 1.0f);
    }
    else
    {
      node->setRect(map_rect);
    }

    if (camera_ && !fit_done_ && map_rect.width() > 0.0 && map_rect.height() > 0.0)
    {
      camera_->fitToRect(map_rect);
      fit_done_ = true;
      emit zoomChanged();
      updateSyncItems();
    }
  }
  else
  {
    node->setRect(map_rect);
  }

  map_transform_node->setMatrix(map_matrix);

  if (camera_)
  {
    camera_->setViewportSize(QSizeF(width(), height()));
    transform_node->setMatrix(camera_->getMatrix());
  }

  map_rect_ = map_rect;
  has_map_rect_ = !map_rect_.isEmpty();

  return transform_node;
}

void MapLayerItem::onFrameReady() { update(); }

QPointF MapLayerItem::clampCenterToMap(const QPointF& center) const
{
  if (!camera_ || !has_map_rect_)
  {
    return center;
  }

  const QRectF rect = map_rect_.normalized();
  if (rect.isEmpty())
  {
    return center;
  }

  const double zoom = camera_->getZoom();
  if (zoom <= 0.0)
  {
    return center;
  }

  const double half_w = width() / (2.0 * zoom);
  const double half_h = height() / (2.0 * zoom);

  double x = center.x();
  double y = center.y();

  double min_x = rect.left() + half_w;
  double max_x = rect.right() - half_w;
  if (rect.width() < (2.0 * half_w))
  {
    min_x = rect.right() - half_w;
    max_x = rect.left() + half_w;
  }
  x = qBound(min_x, x, max_x);

  double min_y = rect.top() + half_h;
  double max_y = rect.bottom() - half_h;
  if (rect.height() < (2.0 * half_h))
  {
    min_y = rect.bottom() - half_h;
    max_y = rect.top() + half_h;
  }
  y = qBound(min_y, y, max_y);

  return QPointF(x, y);
}

void MapLayerItem::applyPanDelta(const QPointF& delta)
{
  if (!camera_)
  {
    return;
  }

  const double zoom = camera_->getZoom();
  if (zoom <= 0.0)
  {
    return;
  }

  const QPointF center = camera_->getViewCenter();
  const QPointF new_center(center.x() - delta.x() / zoom, center.y() + delta.y() / zoom);
  camera_->setViewCenter(clampCenterToMap(new_center));
}

void MapLayerItem::wheelEvent(QWheelEvent* event)
{
  if (!event || !camera_)
  {
    qWarning() << "[MapLayerItem::wheelEvent] Invalid event or camera";
    return;
  }

  const qreal delta = event->angleDelta().y() > 0 ? rendering::RenderCamera::ZOOM_FACTOR : (1.0 / rendering::RenderCamera::ZOOM_FACTOR);
  camera_->zoomByFactor(delta);
  camera_->setViewCenter(clampCenterToMap(camera_->getViewCenter()));
  emit zoomChanged();
  update();
  updateSyncItems();
  event->accept();
}

void MapLayerItem::mousePressEvent(QMouseEvent* event)
{
  if (!event)
  {
    qWarning() << "[MapLayerItem::mousePressEvent] Invalid event";
    return;
  }

  if (!panning_enabled_)
  {
    QQuickItem::mousePressEvent(event);
    return;
  }

  if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)
  {
    is_panning_ = true;
    last_pan_pos_ = event->position();
    event->accept();
    return;
  }

  QQuickItem::mousePressEvent(event);
}

void MapLayerItem::mouseMoveEvent(QMouseEvent* event)
{
  if (!event || !camera_ || !is_panning_ || !panning_enabled_)
  {
    QQuickItem::mouseMoveEvent(event);
    return;
  }

  const QPointF delta = event->position() - last_pan_pos_;
  last_pan_pos_ = event->position();
  applyPanDelta(delta);
  update();
  updateSyncItems();
  event->accept();
}

void MapLayerItem::mouseReleaseEvent(QMouseEvent* event)
{
  if (!event)
  {
    qWarning() << "[MapLayerItem::mouseReleaseEvent] Invalid event";
    return;
  }

  if (!panning_enabled_)
  {
    QQuickItem::mouseReleaseEvent(event);
    return;
  }

  if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)
  {
    is_panning_ = false;
    event->accept();
    return;
  }

  QQuickItem::mouseReleaseEvent(event);
}

void MapLayerItem::touchEvent(QTouchEvent* event)
{
  if (!event)
  {
    qWarning() << "[MapLayerItem::touchEvent] Invalid event";
    return;
  }

  if (!camera_)
  {
    QQuickItem::touchEvent(event);
    return;
  }

  const auto points = event->points();
  if (points.size() == 1)
  {
    is_pinching_ = false;
    if (!panning_enabled_)
    {
      QQuickItem::touchEvent(event);
      return;
    }
    const QPointF pos = points.front().position();
    switch (event->type())
    {
      case QEvent::TouchBegin:
        is_panning_ = true;
        last_pan_pos_ = pos;
        event->accept();
        return;
      case QEvent::TouchUpdate:
        if (is_panning_)
        {
          const QPointF delta = pos - last_pan_pos_;
          last_pan_pos_ = pos;
          applyPanDelta(delta);
          update();
          updateSyncItems();
          event->accept();
          return;
        }
        break;
      case QEvent::TouchEnd:
      case QEvent::TouchCancel:
        is_panning_ = false;
        event->accept();
        return;
      default:
        break;
    }
  }
  else if (points.size() >= 2)
  {
    is_panning_ = false;
    const QLineF line(points[0].position(), points[1].position());
    const qreal distance = line.length();
    if (!is_pinching_ || event->type() == QEvent::TouchBegin)
    {
      pinch_start_distance_ = distance;
      pinch_start_zoom_ = camera_->getZoom();
      is_pinching_ = true;
      event->accept();
      return;
    }

    if (pinch_start_distance_ > 0.0)
    {
      const qreal scale = distance / pinch_start_distance_;
      camera_->setZoom(pinch_start_zoom_ * scale);
      camera_->setViewCenter(clampCenterToMap(camera_->getViewCenter()));
      emit zoomChanged();
      update();
      updateSyncItems();
      event->accept();
      return;
    }
  }
  else
  {
    is_panning_ = false;
    is_pinching_ = false;
  }

  QQuickItem::touchEvent(event);
}

bool MapLayerItem::event(QEvent* event)
{
  if (event && event->type() == QEvent::Gesture && camera_)
  {
    auto* gesture_event = static_cast<QGestureEvent*>(event);
    if (QGesture* pinch = gesture_event->gesture(Qt::PinchGesture))
    {
      auto* pinch_gesture = static_cast<QPinchGesture*>(pinch);
      if (pinch_gesture->state() == Qt::GestureStarted)
      {
        pinch_start_zoom_ = camera_->getZoom();
      }
      const qreal scale = pinch_gesture->totalScaleFactor();
      camera_->setZoom(pinch_start_zoom_ * scale);
      camera_->setViewCenter(clampCenterToMap(camera_->getViewCenter()));
      emit zoomChanged();
      update();
      updateSyncItems();
      return true;
    }
  }

  return QQuickItem::event(event);
}

void MapLayerItem::updateSyncItems()
{
  for (int i = sync_items_.size() - 1; i >= 0; --i)
  {
    QQuickItem* item = sync_items_[i];

    if (!item)
    {
      sync_items_.removeAt(i);
      continue;
    }

    item->update();
  }
}
