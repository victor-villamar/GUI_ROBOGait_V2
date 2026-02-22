#include "Map/Rendering/MapRenderWidget.hpp"

#include <QCursor>
#include <QDebug>
#include <QGestureEvent>
#include <QGraphicsScene>
#include <QPinchGesture>
#include <QtMath>

using namespace ROBOGait::map::rendering;

MapRenderWidget::MapRenderWidget(QQuickItem* parent) : QQuickPaintedItem(parent)
{

  // Initialize member variables
  zoom_level_ = DEFAULT_ZOOM;
  view_center_ = QPointF(0.0, 0.0);
  is_panning_ = false;
  touch_panning_ = false;
  pinch_start_zoom_ = DEFAULT_ZOOM;

  // Enable antialiasing for smooth rendering
  setAntialiasing(true);

  // Accept mouse events
  setAcceptedMouseButtons(Qt::AllButtons);
  setAcceptHoverEvents(true);

  // Enable touch events
  setAcceptTouchEvents(true);

  // Set default size
  setWidth(1920);
  setHeight(1080);

  qInfo() << "[MapRenderWidget::MapRenderWidget] MapRenderWidget created";
}

void MapRenderWidget::setVisualizationManager(std::shared_ptr<VisualizationManager> manager)
{
  if (visualization_manager_)
  {
    // Disconnect previous manager
    // clang-format off
    disconnect(visualization_manager_.get(),
               &VisualizationManager::frameRendered,
               this,
               &MapRenderWidget::onFrameRendered);
    // clang-format on
  }

  visualization_manager_ = manager;

  if (visualization_manager_)
  {
    // Connect to frame rendered signal
    // clang-format off
    connect(visualization_manager_.get(),
            &VisualizationManager::frameRendered,
            this,
            &MapRenderWidget::onFrameRendered);
    // clang-format on

    qInfo() << "[MapRenderWidget::setVisualizationManager] Connected to VisualizationManager";

    // Fit to view if scene has items
    QGraphicsScene* scene = visualization_manager_->getScene();
    if (scene && !scene->items().isEmpty())
    {
      fitToView();
    }

    update();
  }
}

void MapRenderWidget::paint(QPainter* painter)
{

  if (!visualization_manager_ || !painter)
  {
    qWarning() << "[MapRenderWidget::paint] Missing painter or visualization_manager";
    return;
  }

  QGraphicsScene* scene = visualization_manager_->getScene();
  if (!scene)
  {
    qWarning() << "[MapRenderWidget::paint] No scene available";
    return;
  }

  // Save painter state
  painter->save();

  // Smooth scaling to reduce pixelation when zooming
  painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

  // Clear background
  painter->fillRect(0, 0, width(), height(), QColor(50, 50, 50));

  // Calculate visible scene rect in scene coordinates
  QRectF visible_scene_rect;
  visible_scene_rect.setLeft(view_center_.x() - (width() / 2.0) / zoom_level_);
  visible_scene_rect.setRight(view_center_.x() + (width() / 2.0) / zoom_level_);
  visible_scene_rect.setTop(view_center_.y() - (height() / 2.0) / zoom_level_);
  visible_scene_rect.setBottom(view_center_.y() + (height() / 2.0) / zoom_level_);

  QRectF target_rect(0, 0, width(), height());

  scene->render(painter, target_rect, visible_scene_rect, Qt::IgnoreAspectRatio);

  painter->restore();
}

void MapRenderWidget::zoomIn()
{
  double new_zoom = qBound(MIN_ZOOM, zoom_level_ * ZOOM_FACTOR, MAX_ZOOM);
  if (qAbs(new_zoom - zoom_level_) > 1e-6)
  {
    zoom_level_ = new_zoom;
    emit zoomLevelChanged(zoom_level_);
    update();

    // DEBUG
    // qDebug() << "[MapRenderWidget::zoomIn] Zoom level:" << zoom_level_;
  }
}

void MapRenderWidget::zoomOut()
{
  double new_zoom = qBound(MIN_ZOOM, zoom_level_ / ZOOM_FACTOR, MAX_ZOOM);
  if (qAbs(new_zoom - zoom_level_) > 1e-6)
  {
    zoom_level_ = new_zoom;
    emit zoomLevelChanged(zoom_level_);
    update();

    // DEBUG
    // qDebug() << "[MapRenderWidget::zoomOut] Zoom level:" << zoom_level_;
  }
}

void MapRenderWidget::fitToView()
{
  if (!visualization_manager_)
  {
    qCritical() << "[MapRenderWidget::fitToView] No visualization manager set";
    return;
  }

  QGraphicsScene* scene = visualization_manager_->getScene();

  if (!scene)
  {
    qCritical() << "[MapRenderWidget::fitToView] No scene available";
    return;
  }

  QRectF scene_rect = scene->sceneRect();
  if (scene_rect.isEmpty())
  {
    scene_rect = scene->itemsBoundingRect();
  }

  if (scene_rect.isEmpty() || width() <= 0 || height() <= 0)
  {
    qCritical() << "[MapRenderWidget::fitToView] Invalid scene or widget dimensions";
    return;
  }

  // Calculate zoom to fit scene in view with margins
  double margin = 1.1; // 10% margin
  double zoom_x = width() / (scene_rect.width() * margin);
  double zoom_y = height() / (scene_rect.height() * margin);

  zoom_level_ = qBound(MIN_ZOOM, qMin(zoom_x, zoom_y), MAX_ZOOM);
  view_center_ = scene_rect.center();

  emit zoomLevelChanged(zoom_level_);
  update();

  // DEBUG
  // qDebug() << "[MapRenderWidget::fitToView] Zoom:" << zoom_level_ << "Center:" << view_center_;
}

void MapRenderWidget::wheelEvent(QWheelEvent* event)
{
  if (!event)
  {
    qWarning() << "[MapRenderWidget::wheelEvent] Null event pointer";
    return;
  }

  double delta = event->angleDelta().y() > 0 ? ZOOM_FACTOR : 1.0 / ZOOM_FACTOR;

  QPointF mouse_pos = event->position();

  zoomAtPoint(mouse_pos, delta);

  event->accept();
}

void MapRenderWidget::mousePressEvent(QMouseEvent* event)
{
  if (!event)
  {
    return;
  }

  if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)
  {
    is_panning_ = true;
    last_pan_pos_ = event->position();
    setCursor(QCursor(Qt::ClosedHandCursor));
    event->accept();
  }
  else
  {
    QQuickPaintedItem::mousePressEvent(event);
  }
}

void MapRenderWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (!event)
  {
    return;
  }

  if (is_panning_)
  {
    QPointF delta = event->position() - last_pan_pos_;
    QPointF scene_delta = delta / zoom_level_;

    view_center_ -= scene_delta;
    last_pan_pos_ = event->position();

    update();
    event->accept();
  }
  else
  {
    QQuickPaintedItem::mouseMoveEvent(event);
  }
}

void MapRenderWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if (!event)
  {
    return;
  }

  if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)
  {
    is_panning_ = false;
    unsetCursor();
    event->accept();
  }
  else
  {
    QQuickPaintedItem::mouseReleaseEvent(event);
  }
}

bool MapRenderWidget::event(QEvent* event)
{
  if (!event)
  {
    return false;
  }

  if (event->type() == QEvent::Gesture)
  {
    QGestureEvent* gesture_event = static_cast<QGestureEvent*>(event);
    if (QGesture* gesture = gesture_event->gesture(Qt::PinchGesture))
    {
      handlePinchGesture(static_cast<QPinchGesture*>(gesture));
      return true;
    }
  }
  else if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd)
  {
    QTouchEvent* touch_event = static_cast<QTouchEvent*>(event);

    if (touch_event->points().size() == 1)
    {
      const QEventPoint& touch_point = touch_event->points().first();

      if (event->type() == QEvent::TouchBegin)
      {
        touch_panning_ = true;
        last_touch_pos_ = touch_point.position();
      }
      else if (event->type() == QEvent::TouchUpdate && touch_panning_)
      {
        QPointF delta = touch_point.position() - last_touch_pos_;
        QPointF scene_delta = delta / zoom_level_;

        view_center_ -= scene_delta;
        last_touch_pos_ = touch_point.position();

        update();
      }
      else if (event->type() == QEvent::TouchEnd)
      {
        touch_panning_ = false;
      }

      return true;
    }
  }

  return QQuickPaintedItem::event(event);
}

void MapRenderWidget::geometryChange(const QRectF& new_geometry, const QRectF& old_geometry)
{
  QQuickPaintedItem::geometryChange(new_geometry, old_geometry);

  if (new_geometry.size() != old_geometry.size())
  {
    // DEBUG
    // qDebug() << "[MapRenderWidget::geometryChange] New size:" << new_geometry.size();
    update();
  }
}

void MapRenderWidget::onFrameRendered(double fps)
{
  static bool initial_fit_done = false;

  // Auto fit to view on first frame with items
  if (!initial_fit_done && visualization_manager_)
  {
    QGraphicsScene* scene = visualization_manager_->getScene();
    if (scene && !scene->items().isEmpty())
    {
      fitToView();
      initial_fit_done = true;
      qInfo() << "[MapRenderWidget::onFrameRendered] Auto fit-to-view applied on first frame with items";
    }
  }

  update();
}

void MapRenderWidget::zoomAtPoint(const QPointF& center_pos, double zoom_delta)
{
  // Convert item coordinates to scene coordinates before zoom
  QPointF item_center = QPointF(width() / 2.0, height() / 2.0);
  QPointF offset_from_center = center_pos - item_center;
  QPointF scene_pos_before = view_center_ + (offset_from_center / zoom_level_);

  double new_zoom = qBound(MIN_ZOOM, zoom_level_ * zoom_delta, MAX_ZOOM);

  if (qAbs(new_zoom - zoom_level_) < 1e-6)
  {
    return; // No change
  }

  QPointF scene_pos_after_desired = scene_pos_before;
  QPointF offset_after = offset_from_center / new_zoom;
  view_center_ = scene_pos_after_desired - offset_after;

  zoom_level_ = new_zoom;

  emit zoomLevelChanged(zoom_level_);
  update();
}

void MapRenderWidget::handlePinchGesture(QPinchGesture* gesture)
{
  if (!gesture)
  {
    return;
  }

  if (gesture->state() == Qt::GestureStarted)
  {
    pinch_start_zoom_ = zoom_level_;
  }
  else if (gesture->state() == Qt::GestureUpdated)
  {
    double scale_factor = gesture->totalScaleFactor();
    double new_zoom = qBound(MIN_ZOOM, pinch_start_zoom_ * scale_factor, MAX_ZOOM);

    if (qAbs(new_zoom - zoom_level_) > 1e-6)
    {
      // Get pinch center
      QPointF center = gesture->centerPoint();

      // Apply zoom at pinch center
      zoomAtPoint(center, new_zoom / zoom_level_);
    }
  }
}
