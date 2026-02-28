#pragma once

#include <QMatrix4x4>
#include <QMutex>
#include <QPointF>
#include <QSizeF>

namespace ROBOGait
{
namespace map
{
namespace rendering
{

/**
 * @brief Manages the view transformation for rendering a map (zoom/center/viewport)
 *
 */
class RenderCamera
{
public:
  RenderCamera();

  /**
   * @brief Set the viewport size
   *
   * @param size New viewport size
   */
  void setViewportSize(const QSizeF& size);

  /**
   * @brief Set the view center
   *
   * @param center New view center
   */
  void setViewCenter(const QPointF& center);

  /**
   * @brief Get the view center
   *
   * @return Current view center
   */
  QPointF getViewCenter() const;

  /**
   * @brief Set the zoom level
   *
   * @param zoom New zoom level
   */
  void setZoom(double zoom);
  /**
   * @brief Get the current zoom level
   *
   * @return Current zoom level
   */
  double getZoom() const;

  /**
   * @brief Zoom the camera by a factor
   *
   * @param factor Zoom factor (e.g., 1.1 to zoom in, 0.9 to zoom out)
   */
  void zoomByFactor(double factor);

  /**
   * @brief Fit the camera to a rectangle
   *
   * @param rect Rectangle to fit the camera to
   * @param margin Margin to apply around the rectangle (default: 1.1 for 10% margin)
   */
  void fitToRect(const QRectF& rect, double margin = 1.1);

  /**
   * @brief Get the current transformation matrix
   *
   * @return Current transformation matrix
   */
  QMatrix4x4 getMatrix() const;

private:
  mutable QMutex mutex_; /**< Mutex for thread-safe access */
  QSizeF viewport_;      /**< Current viewport size */
  QPointF center_;       /**< Current view center */
  double zoom_;          /**< Current zoom level */
};

} // namespace rendering
} // namespace map
} // namespace ROBOGait
