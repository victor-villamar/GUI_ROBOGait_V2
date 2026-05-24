#pragma once

#include <QPointF>
#include <QVector>

#include "Map/Interaction/Geometry/BezierSplineModel.hpp"

namespace ROBOGait
{
namespace map
{
namespace interaction
{
namespace geometry
{
/**
 * @brief Converts sampled polylines into editable cubic Bezier chains
 */
class SplineFitter
{
public:
  /**
   * @brief Fit centripetal Catmull-Rom and convert to cubic Bezier segments
   */
  static QVector<CubicBezierSegment> fitCentripetalCatmullRom(const QVector<QPointF>& points, double alpha);

private:
  static QPointF safeTangent(const QPointF& previous, const QPointF& next);
  static double nextParameter(double previous_t, const QPointF& p0, const QPointF& p1, double alpha);
  static QPointF interpolateDerivative(const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, double t0, double t1, double t2, double t3,
                                       bool first_derivative);
};
} // namespace geometry
} // namespace interaction
} // namespace map
} // namespace ROBOGait
