#include <QDebug>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QSGTexture>
#include <QSGTransformNode>

#include "Map/Items/MapLayerItem.hpp"

using namespace ROBOGait::map::item;

MapLayerItem::MapLayerItem(QQuickItem* parent) : QQuickItem(parent), last_image_key_(0), fit_done_(false), is_panning_(false), pinch_start_zoom_(1.0)
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

  QSGSimpleTextureNode* node = nullptr;
  if (!transform_node->firstChild())
  {
    node = new QSGSimpleTextureNode();
    transform_node->appendChildNode(node);
  }
  else
  {
    node = static_cast<QSGSimpleTextureNode*>(transform_node->firstChild());
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
  if (map_render_->getMapData())
  {
    auto metadata = map_render_->getMapData()->getMetadata();
    const double width_m = static_cast<double>(metadata.width) * metadata.resolution;
    const double height_m = static_cast<double>(metadata.height) * metadata.resolution;
    const double origin_x = metadata.origin_x;
    const double origin_y = -(metadata.origin_y + height_m);
    map_rect = QRectF(origin_x, origin_y, width_m, height_m);

    if (camera_ && !fit_done_ && width_m > 0.0 && height_m > 0.0)
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

  node->setRect(map_rect);

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
  if (!event || !camera_ || !is_panning_)
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
