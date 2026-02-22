#pragma once

#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QPinchGesture>
#include <QQuickPaintedItem>
#include <QTouchEvent>
#include <QWheelEvent>
#include <memory>

#include "Map/Rendering/VisualizationManager.hpp"

namespace ROBOGait
{
namespace map
{
namespace rendering
{

/**
 * @brief QQuickPaintedItem-based widget for 2D map visualization with touch support
 *
 * This widget integrates QGraphicsScene rendering into QML using QQuickPaintedItem.
 * It provides dual input support for development (mouse) and production (tablet):
 *
 * **Mouse Input (development)**:
 * - Zoom: Mouse wheel (centered on cursor position)
 * - Pan: Middle mouse button drag
 *
 * **Touch Input (tablet)**:
 * - Zoom: Pinch gesture
 * - Pan: Single finger drag
 *
 */
class MapRenderWidget : public QQuickPaintedItem
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of MapRenderWidget class
   *
   * @param parent Qt parent item
   */
  MapRenderWidget(QQuickItem* parent = nullptr);

  /**
   * @brief Destructor of MapRenderWidget class
   */
  ~MapRenderWidget() override = default;

  /**
   * @brief Set the visualization manager to render its scene
   *
   * @param manager Shared pointer to the visualization manager
   */
  void setVisualizationManager(std::shared_ptr<VisualizationManager> manager);

  /**
   * @brief Paint the QGraphicsScene content onto the QML item
   *
   * @param painter QPainter used for rendering
   */
  void paint(QPainter* painter) override;

  /**
   * @brief Get current zoom level
   *
   * @return Zoom level (1.0 = 100%)
   */
  double getZoomLevel() const { return zoom_level_; }

  /**
   * @brief Zoom in by one step (15%)
   */
  Q_INVOKABLE void zoomIn();

  /**
   * @brief Zoom out by one step (15%)
   */
  Q_INVOKABLE void zoomOut();

  /**
   * @brief Fit view to scene contents
   */
  Q_INVOKABLE void fitToView();

signals:
  void zoomLevelChanged(double zoom); // Signal emitted when zoom level changes

protected:
  /**
   * @brief Handle mouse wheel events for zooming
   *
   * @param event Mouse wheel event
   */
  void wheelEvent(QWheelEvent* event) override;

  /**
   * @brief Handle mouse press events for panning
   *
   * @param event Mouse press event
   */
  void mousePressEvent(QMouseEvent* event) override;

  /**
   * @brief Handle mouse move events for panning
   *
   * @param event Mouse move event
   */
  void mouseMoveEvent(QMouseEvent* event) override;

  /**
   * @brief Handle mouse release events for panning
   *
   * @param event Mouse release event
   */
  void mouseReleaseEvent(QMouseEvent* event) override;

  /**
   * @brief Handle events including gestures
   *
   * @param event Event
   */
  bool event(QEvent* event) override;

  /**
   * @brief Handle geometry changes
   *
   * @param new_geometry New geometry
   * @param old_geometry Old geometry
   */
  void geometryChange(const QRectF& new_geometry, const QRectF& old_geometry) override;

private slots:
  void onFrameRendered(double fps); // Handle frame rendered signal from VisualizationManager

private:
  /**
   * @brief Zoom centered on a specific point
   *
   * @param center_pos Point to center zoom on (item coordinates)
   * @param zoom_delta Zoom factor (e.g., 1.15 = 15% zoom in)
   */
  void zoomAtPoint(const QPointF& center_pos, double zoom_delta);

  /**
   * @brief Handle pinch gesture for zooming
   *
   * @param gesture Pinch gesture
   */
  void handlePinchGesture(QPinchGesture* gesture);

  std::shared_ptr<VisualizationManager> visualization_manager_; /**< Visualization manager */

  double zoom_level_;   /**< Current zoom level (1.0 = 100%) */
  QPointF view_center_; /**< View center in scene coordinates */

  bool is_panning_;      /**< Whether currently panning with mouse */
  QPointF last_pan_pos_; /**< Last mouse position during panning */

  bool touch_panning_;      /**< Whether currently panning with touch */
  QPointF last_touch_pos_;  /**< Last touch position during pan */
  double pinch_start_zoom_; /**< Zoom level at pinch start */

  static constexpr double DEFAULT_ZOOM = 1.0; /**< Default zoom level (1.0 = 100%) */
  static constexpr double MIN_ZOOM = 0.01;    /**< Minimum zoom (1%) - far out view */
  static constexpr double MAX_ZOOM = 1000.0;  /**< Maximum zoom (100000%) - close up view */
  static constexpr double ZOOM_FACTOR = 1.15; /**< Zoom increment per wheel step */
};

} // namespace rendering
} // namespace map
} // namespace ROBOGait
