#include <cmath>

#include <QLineF>
#include <QtGlobal>

#include "Map/Interaction/Geometry/SplineFitter.hpp"

namespace ROBOGait
{
namespace map
{
namespace interaction
{
namespace geometry
{
namespace
{
QPointF scalePoint(const QPointF& point, double factor) { return QPointF(point.x() * factor, point.y() * factor); }
} // namespace

QVector<CubicBezierSegment> SplineFitter::fitCentripetalCatmullRom(const QVector<QPointF>& points, double alpha)
{
  QVector<CubicBezierSegment> segments;

  if (points.size() < 2)
  {
    return segments;
  }

  const double clamped_alpha = qBound(0.0, alpha, 1.0);

  if (points.size() == 2)
  {
    const QPointF direction = points[1] - points[0];

    CubicBezierSegment segment;
    segment.p0 = points[0];
    segment.c1 = points[0] + scalePoint(direction, 1.0 / 3.0);
    segment.c2 = points[0] + scalePoint(direction, 2.0 / 3.0);
    segment.p1 = points[1];
    segments.append(segment);
    return segments;
  }

  segments.reserve(points.size() - 1);

  for (int i = 0; i + 1 < points.size(); ++i)
  {
    const QPointF p0 = points[qMax(0, i - 1)];
    const QPointF p1 = points[i];
    const QPointF p2 = points[i + 1];
    const QPointF p3 = points[qMin(points.size() - 1, i + 2)];

    const double t0 = 0.0;
    const double t1 = nextParameter(t0, p0, p1, clamped_alpha);
    const double t2 = nextParameter(t1, p1, p2, clamped_alpha);
    const double t3 = nextParameter(t2, p2, p3, clamped_alpha);

    QPointF d1 = interpolateDerivative(p0, p1, p2, p3, t0, t1, t2, t3, true);
    QPointF d2 = interpolateDerivative(p0, p1, p2, p3, t0, t1, t2, t3, false);

    if (!std::isfinite(d1.x()) || !std::isfinite(d1.y()))
    {
      d1 = safeTangent(p0, p2);
    }
    if (!std::isfinite(d2.x()) || !std::isfinite(d2.y()))
    {
      d2 = safeTangent(p1, p3);
    }

    CubicBezierSegment segment;
    segment.p0 = p1;
    segment.c1 = p1 + scalePoint(d1, 1.0 / 3.0);
    segment.c2 = p2 - scalePoint(d2, 1.0 / 3.0);
    segment.p1 = p2;
    segments.append(segment);
  }

  return segments;
}

QPointF SplineFitter::safeTangent(const QPointF& previous, const QPointF& next) { return scalePoint(next - previous, 0.5); }

double SplineFitter::nextParameter(double previous_t, const QPointF& p0, const QPointF& p1, double alpha)
{
  const double distance = QLineF(p0, p1).length();
  return previous_t + std::pow(qMax(distance, 1e-9), alpha);
}

QPointF SplineFitter::interpolateDerivative(const QPointF& p0, const QPointF& p1, const QPointF& p2, const QPointF& p3, double t0, double t1, double t2,
                                            double t3, bool first_derivative)
{
  const double eps = 1e-9;
  const double t21 = qMax(t2 - t1, eps);
  const double t10 = qMax(t1 - t0, eps);
  const double t20 = qMax(t2 - t0, eps);
  const double t32 = qMax(t3 - t2, eps);
  const double t31 = qMax(t3 - t1, eps);

  if (first_derivative)
  {
    const QPointF a = (p1 - p0) / t10;
    const QPointF b = (p2 - p0) / t20;
    const QPointF c = (p2 - p1) / t21;
    return (a - b + c) * t21;
  }

  const QPointF a = (p2 - p1) / t21;
  const QPointF b = (p3 - p1) / t31;
  const QPointF c = (p3 - p2) / t32;
  return (a - b + c) * t21;
}
} // namespace geometry
} // namespace interaction
} // namespace map
} // namespace ROBOGait
