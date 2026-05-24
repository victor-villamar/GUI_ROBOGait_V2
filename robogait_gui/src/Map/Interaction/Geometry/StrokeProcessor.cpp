#include <QLineF>
#include <QtGlobal>

#include "Map/Interaction/Geometry/StrokeProcessor.hpp"

namespace ROBOGait
{
namespace map
{
namespace interaction
{
namespace geometry
{
QVector<QPointF> StrokeProcessor::removeDuplicatedPoints(const QVector<QPointF>& points, double min_distance_m)
{
  if (points.size() < MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  const double threshold = qMax(min_distance_m, 1e-9);
  QVector<QPointF> filtered;
  filtered.reserve(points.size());
  filtered.append(points.constFirst());

  for (int i = 1; i < points.size(); ++i)
  {
    if (QLineF(filtered.constLast(), points[i]).length() >= threshold)
    {
      filtered.append(points[i]);
    }
  }

  return filtered;
}

QVector<QPointF> StrokeProcessor::resampleByArcLength(const QVector<QPointF>& points, double spacing_m)
{
  if (points.size() < MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  const double spacing = qMax(spacing_m, 1e-6);
  QVector<QPointF> out;
  out.reserve(points.size());
  out.append(points.constFirst());

  double distance_until_next_sample = spacing;

  for (int i = 1; i < points.size(); ++i)
  {
    const QPointF& segment_start = points[i - 1];
    const QPointF& segment_end = points[i];
    const QLineF segment(segment_start, segment_end);
    const double segment_length = segment.length();

    if (segment_length <= 1e-9)
    {
      continue;
    }

    const QPointF direction((segment_end.x() - segment_start.x()) / segment_length, (segment_end.y() - segment_start.y()) / segment_length);
    double consumed = 0.0;

    while ((consumed + distance_until_next_sample) <= (segment_length + 1e-9))
    {
      consumed += distance_until_next_sample;
      const QPointF sample(segment_start.x() + direction.x() * consumed, segment_start.y() + direction.y() * consumed);

      if (QLineF(out.constLast(), sample).length() > 1e-9)
      {
        out.append(sample);
      }

      distance_until_next_sample = spacing;
    }

    distance_until_next_sample -= (segment_length - consumed);
    if (distance_until_next_sample <= 1e-9)
    {
      distance_until_next_sample = spacing;
    }
  }

  if (QLineF(out.constLast(), points.constLast()).length() > 1e-9)
  {
    out.append(points.constLast());
  }

  return out;
}

QVector<QPointF> StrokeProcessor::smoothMovingAverage(const QVector<QPointF>& points, int window_radius)
{
  if (points.size() < MIN_VALID_PATH_POINTS || window_radius <= 0)
  {
    return points;
  }

  const int point_count = static_cast<int>(points.size());
  QVector<QPointF> smoothed(points.size());
  smoothed[0] = points[0];
  smoothed[point_count - 1] = points[point_count - 1];

  for (int i = 1; i < point_count - 1; ++i)
  {
    const int first = qMax(0, i - window_radius);
    const int max_index = point_count - 1;
    const int last = qMin(max_index, i + window_radius);

    double sum_x = 0.0;
    double sum_y = 0.0;
    int count = 0;

    for (int j = first; j <= last; ++j)
    {
      sum_x += points[j].x();
      sum_y += points[j].y();
      ++count;
    }

    if (count > 0)
    {
      smoothed[i] = QPointF(sum_x / static_cast<double>(count), sum_y / static_cast<double>(count));
    }
    else
    {
      smoothed[i] = points[i];
    }
  }

  return smoothed;
}

QVector<QPointF> StrokeProcessor::simplifyDouglasPeucker(const QVector<QPointF>& points, double tolerance_m)
{
  if (points.size() < MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  const double tolerance = qMax(tolerance_m, 1e-9);
  const int point_count = static_cast<int>(points.size());
  QVector<bool> keep(point_count, false);
  keep[0] = true;
  keep[point_count - 1] = true;

  struct SegmentRange
  {
    int first;
    int last;
  };

  QVector<SegmentRange> stack;
  stack.reserve(point_count);
  stack.append({0, point_count - 1});

  while (!stack.isEmpty())
  {
    const SegmentRange range = stack.constLast();
    stack.removeLast();

    if (range.last <= (range.first + 1))
    {
      continue;
    }

    const QPointF a = points[range.first];
    const QPointF b = points[range.last];
    const QLineF baseline(a, b);
    const double baseline_length = baseline.length();

    int farthest_index = -1;
    double farthest_distance = 0.0;

    for (int i = range.first + 1; i < range.last; ++i)
    {
      const QPointF p = points[i];
      double distance = 0.0;

      if (baseline_length <= 1e-12)
      {
        distance = QLineF(a, p).length();
      }
      else
      {
        const double t = qBound(0.0, ((p.x() - a.x()) * (b.x() - a.x()) + (p.y() - a.y()) * (b.y() - a.y())) / (baseline_length * baseline_length), 1.0);
        const QPointF projection(a.x() + t * (b.x() - a.x()), a.y() + t * (b.y() - a.y()));
        distance = QLineF(projection, p).length();
      }

      if (distance > farthest_distance)
      {
        farthest_distance = distance;
        farthest_index = i;
      }
    }

    if (farthest_index >= 0 && farthest_distance > tolerance)
    {
      keep[farthest_index] = true;
      stack.append({range.first, farthest_index});
      stack.append({farthest_index, range.last});
    }
  }

  QVector<QPointF> simplified;
  simplified.reserve(point_count);

  for (int i = 0; i < point_count; ++i)
  {
    if (keep[i])
    {
      simplified.append(points[i]);
    }
  }

  if (simplified.size() < MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  return simplified;
}

QVector<QPointF> StrokeProcessor::limitPointCount(const QVector<QPointF>& points, int max_points)
{
  if (points.size() <= MIN_VALID_PATH_POINTS || max_points < MIN_VALID_PATH_POINTS || points.size() <= max_points)
  {
    return points;
  }

  QVector<QPointF> reduced;
  reduced.reserve(max_points);
  reduced.append(points.constFirst());

  const int points_size = static_cast<int>(points.size());
  const int interior_source = points_size - 2;
  const int interior_target = max_points - 2;
  if (interior_target > 0 && interior_source > 0)
  {
    for (int i = 1; i <= interior_target; ++i)
    {
      const int source_index =
          1 + static_cast<int>((static_cast<long long>(i) * static_cast<long long>(interior_source)) / static_cast<long long>(interior_target + 1));
      const int max_interior_index = points_size - 2;
      const int clamped_index = qBound(1, source_index, max_interior_index);
      reduced.append(points[clamped_index]);
    }
  }

  reduced.append(points.constLast());
  return removeDuplicatedPoints(reduced, 1e-9);
}
} // namespace geometry
} // namespace interaction
} // namespace map
} // namespace ROBOGait
