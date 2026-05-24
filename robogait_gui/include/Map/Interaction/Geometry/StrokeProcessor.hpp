#pragma once

#include <QPointF>
#include <QVector>

namespace ROBOGait
{
namespace map
{
namespace interaction
{
namespace geometry
{
/**
 * @brief Stateless preprocessing utilities for freehand stroke samples
 */
class StrokeProcessor
{
public:
  /**
   * @brief Remove consecutive duplicates or near-duplicates
   */
  static QVector<QPointF> removeDuplicatedPoints(const QVector<QPointF>& points, double min_distance_m);

  /**
   * @brief Uniformly resample a polyline by arc length spacing
   */
  static QVector<QPointF> resampleByArcLength(const QVector<QPointF>& points, double spacing_m);

  /**
   * @brief Smooth points with centered moving average while preserving endpoints
   */
  static QVector<QPointF> smoothMovingAverage(const QVector<QPointF>& points, int window_radius);

  /**
   * @brief Simplify a polyline using Douglas-Peucker tolerance
   */
  static QVector<QPointF> simplifyDouglasPeucker(const QVector<QPointF>& points, double tolerance_m);

  /**
   * @brief Reduce point count with uniform downsampling while preserving endpoints
   */
  static QVector<QPointF> limitPointCount(const QVector<QPointF>& points, int max_points);

private:
  static constexpr int MIN_VALID_PATH_POINTS = 2; /**< Minimum number of points required to form a path */
};
} // namespace geometry
} // namespace interaction
} // namespace map
} // namespace ROBOGait
