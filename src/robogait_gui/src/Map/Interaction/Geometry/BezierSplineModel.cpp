#include <cmath>

#include <QLineF>
#include <QtGlobal>

#include "Map/Interaction/Geometry/BezierSplineModel.hpp"

namespace ROBOGait
{
namespace map
{
namespace interaction
{
namespace geometry
{
void BezierSplineModel::clear()
{
  anchors_.clear();
  handles_in_.clear();
  handles_out_.clear();
}

bool BezierSplineModel::setFromSegments(const QVector<CubicBezierSegment>& segments)
{
  clear();

  if (segments.size() < MIN_VALID_SEGMENTS)
  {
    return false;
  }

  anchors_.reserve(segments.size() + 1);
  handles_in_.reserve(segments.size() + 1);
  handles_out_.reserve(segments.size() + 1);

  anchors_.append(segments.constFirst().p0);
  handles_in_.append(segments.constFirst().p0);
  handles_out_.append(segments.constFirst().c1);

  for (int i = 0; i < segments.size(); ++i)
  {
    const CubicBezierSegment& segment = segments[i];
    anchors_.append(segment.p1);
    handles_in_.append(segment.c2);
    handles_out_.append(segment.p1);

    if (i + 1 < segments.size())
    {
      handles_out_[i + 1] = segments[i + 1].c1;
    }
  }

  return isValid();
}

QVector<CubicBezierSegment> BezierSplineModel::getSegments() const
{
  QVector<CubicBezierSegment> segments;

  if (!isValid())
  {
    return segments;
  }

  segments.reserve(anchors_.size() - 1);

  for (int i = 0; i + 1 < anchors_.size(); ++i)
  {
    CubicBezierSegment segment;
    segment.p0 = anchors_[i];
    segment.c1 = handles_out_[i];
    segment.c2 = handles_in_[i + 1];
    segment.p1 = anchors_[i + 1];
    segments.append(segment);
  }

  return segments;
}

QVector<BezierSplineModel::ControlPointRef> BezierSplineModel::getControlPoints() const
{
  QVector<ControlPointRef> points;

  if (!isValid())
  {
    return points;
  }

  points.reserve(anchors_.size() * 3);

  for (int i = 0; i < anchors_.size(); ++i)
  {
    points.append({ControlPointType::Anchor, i, anchors_[i]});
    points.append({ControlPointType::HandleIn, i, handles_in_[i]});
    points.append({ControlPointType::HandleOut, i, handles_out_[i]});
  }

  return points;
}

bool BezierSplineModel::setControlPoint(ControlPointType type, int index, const QPointF& position)
{
  if (!isValid())
  {
    return false;
  }

  if (index < 0 || index >= anchors_.size())
  {
    return false;
  }

  switch (type)
  {
    case ControlPointType::Anchor:
    {
      const QPointF delta = position - anchors_[index];
      anchors_[index] = position;
      handles_in_[index] += delta;
      handles_out_[index] += delta;
      break;
    }
    case ControlPointType::HandleIn:
    {
      handles_in_[index] = position;
      break;
    }
    case ControlPointType::HandleOut:
    {
      handles_out_[index] = position;
      break;
    }
    default:
    {
      return false;
    }
  }

  return true;
}

QVector<QPointF> BezierSplineModel::sampleByDistance(double spacing_m) const
{
  QVector<QPointF> sampled;

  if (!isValid())
  {
    return sampled;
  }

  const QVector<CubicBezierSegment> segments = getSegments();
  const double spacing = qMax(spacing_m, 1e-6);

  for (int i = 0; i < segments.size(); ++i)
  {
    const CubicBezierSegment& segment = segments[i];
    const double length = approximateLength(segment, LENGTH_SUBDIVISIONS);
    const int subdivisions = qMax(1, static_cast<int>(std::ceil(length / spacing)));
    const int start_step = (i == 0) ? 0 : 1;

    for (int step = start_step; step <= subdivisions; ++step)
    {
      const double t = static_cast<double>(step) / static_cast<double>(subdivisions);
      sampled.append(evaluateCubic(segment, t));
    }
  }

  return sampled;
}

bool BezierSplineModel::isValid() const
{
  if (anchors_.size() < 2)
  {
    return false;
  }

  if (handles_in_.size() != anchors_.size() || handles_out_.size() != anchors_.size())
  {
    return false;
  }

  return true;
}

QPointF BezierSplineModel::evaluateCubic(const CubicBezierSegment& segment, double t)
{
  const double clamped_t = qBound(0.0, t, 1.0);
  const double one_minus_t = 1.0 - clamped_t;
  const double b0 = one_minus_t * one_minus_t * one_minus_t;
  const double b1 = 3.0 * one_minus_t * one_minus_t * clamped_t;
  const double b2 = 3.0 * one_minus_t * clamped_t * clamped_t;
  const double b3 = clamped_t * clamped_t * clamped_t;

  return QPointF((segment.p0.x() * b0) + (segment.c1.x() * b1) + (segment.c2.x() * b2) + (segment.p1.x() * b3),
                 (segment.p0.y() * b0) + (segment.c1.y() * b1) + (segment.c2.y() * b2) + (segment.p1.y() * b3));
}

double BezierSplineModel::approximateLength(const CubicBezierSegment& segment, int subdivisions)
{
  const int safe_subdivisions = qMax(1, subdivisions);
  double length = 0.0;
  QPointF previous = evaluateCubic(segment, 0.0);

  for (int i = 1; i <= safe_subdivisions; ++i)
  {
    const double t = static_cast<double>(i) / static_cast<double>(safe_subdivisions);
    const QPointF current = evaluateCubic(segment, t);
    length += QLineF(previous, current).length();
    previous = current;
  }

  return length;
}
} // namespace geometry
} // namespace interaction
} // namespace map
} // namespace ROBOGait
