#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QtMath>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QSGTexture>
#include <QSGTransformNode>

#include "Map/Items/MapLayerItem.hpp"

using namespace ROBOGait::map::item;

MapLayerItem::MapLayerItem(QQuickItem* parent) : QQuickItem(parent), last_image_key_(0), fit_done_(false), is_panning_(false), panning_enabled_(true), pinch_start_zoom_(1.0)
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

void MapLayerItem::setSyncItem(QQuickItem* item) { sync_item_ = item; }

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

      auto mapPoint = [&](double x, double y) {
        return QPointF(origin_x + cos_t * x - sin_t * y, origin_y + sin_t * x + cos_t * y);
      };

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
      if (sync_item_)
      {
        sync_item_->update();
      }
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

  return transform_node;
}

void MapLayerItem::onFrameReady() { update(); }

void MapLayerItem::wheelEvent(QWheelEvent* event)
{
  if (!event || !camera_)
  {
    qWarning() << "[MapLayerItem::wheelEvent] Invalid event or camera";
    return;
  }

  const qreal delta = event->angleDelta().y() > 0 ? rendering::RenderCamera::ZOOM_FACTOR : (1.0 / rendering::RenderCamera::ZOOM_FACTOR);
  camera_->zoomByFactor(delta);
  emit zoomChanged();
  update();
  if (sync_item_)
  {
    sync_item_->update();
  }
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

  const QPointF center = camera_->getViewCenter();
  const double zoom = camera_->getZoom();
  const QPointF new_center(center.x() - delta.x() / zoom, center.y() - delta.y() / zoom);
  camera_->setViewCenter(new_center);
  update();
  if (sync_item_)
  {
    sync_item_->update();
  }
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
      const qreal scale = pinch_gesture->scaleFactor();
      camera_->setZoom(pinch_start_zoom_ * scale);
      emit zoomChanged();
      update();
      if (sync_item_)
      {
        sync_item_->update();
      }
      return true;
    }
  }

  return QQuickItem::event(event);
}
