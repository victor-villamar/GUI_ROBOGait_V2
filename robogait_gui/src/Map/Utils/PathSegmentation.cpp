#include <algorithm>
#include <functional>
#include <limits>
#include <vector>

#include <QLineF>
#include <QSet>
#include <QtGlobal>

#include "Map/Utils/PathSegmentation.hpp"

using namespace ROBOGait::map::utils;

QVector<QPointF> PathSegmentation::segmentPolyline(const QVector<QPointF>& points, const Config& config)
{
  if (points.size() <= MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  const double spacing_m = qMax(config.resample_spacing_m, 1e-6);
  const double epsilon_m = qMax(config.rdp_epsilon_m, 1e-6);

  const QVector<QPointF> uniform_points = uniformResample(points, spacing_m);

  if (uniform_points.size() <= MIN_VALID_PATH_POINTS)
  {
    return uniform_points;
  }

  const QVector<int> corners =
      detectShortStrawCorners(uniform_points, config.short_straw_window, config.short_straw_median_factor, config.short_straw_line_threshold);
  return simplifyWithLockedCorners(uniform_points, corners, epsilon_m);
}

QVector<QPointF> PathSegmentation::uniformResample(const QVector<QPointF>& points, double spacing_m)
{
  if (points.size() <= 1 || spacing_m <= 1e-9)
  {
    return points;
  }

  QVector<QPointF> out;
  out.reserve(points.size());
  out.append(points.constFirst());

  double remaining_until_next_sample = spacing_m;

  for (int i = 1; i < static_cast<int>(points.size()); ++i)
  {
    const QPointF& segment_start = points[i - 1];
    const QPointF& segment_end = points[i];
    const QLineF segment(segment_start, segment_end);
    const double segment_length = segment.length();

    if (segment_length <= 1e-9)
    {
      continue;
    }

    const qreal direction_x = (segment_end.x() - segment_start.x()) / segment_length;
    const qreal direction_y = (segment_end.y() - segment_start.y()) / segment_length;

    const QPointF direction(direction_x, direction_y);
    double consumed = 0.0;

    while ((consumed + remaining_until_next_sample) <= (segment_length + 1e-9))
    {
      consumed += remaining_until_next_sample;

      const qreal sample_x = segment_start.x() + direction.x() * consumed;
      const qreal sample_y = segment_start.y() + direction.y() * consumed;

      const QPointF sample(sample_x, sample_y);

      if (QLineF(out.constLast(), sample).length() > 1e-9)
      {
        out.append(sample);
      }

      remaining_until_next_sample = spacing_m;
    }

    remaining_until_next_sample -= (segment_length - consumed);

    if (remaining_until_next_sample <= 1e-9)
    {
      remaining_until_next_sample = spacing_m;
    }
  }

  if (QLineF(out.constLast(), points.constLast()).length() > 1e-9)
  {
    out.append(points.constLast());
  }

  return out;
}

QVector<int> PathSegmentation::detectShortStrawCorners(const QVector<QPointF>& points, int window, double median_factor, double line_threshold)
{
  QVector<int> corners;
  const int point_count = static_cast<int>(points.size());

  if (point_count == 0)
  {
    return corners;
  }

  if (point_count == 1)
  {
    corners.append(0);
    return corners;
  }

  const int window_size = qMax(1, window);
  if (point_count <= (2 * window_size + 1))
  {
    corners.append(0);
    corners.append(point_count - 1);
    return corners;
  }

  QVector<double> straw(point_count, std::numeric_limits<double>::infinity());
  std::vector<double> straw_values;
  straw_values.reserve(static_cast<size_t>(point_count));

  for (int i = window_size; i < (point_count - window_size); ++i)
  {
    const double value = QLineF(points[i - window_size], points[i + window_size]).length();
    straw[i] = value;
    straw_values.push_back(value);
  }

  if (straw_values.empty())
  {
    corners.append(0);
    corners.append(point_count - 1);
    return corners;
  }

  std::sort(straw_values.begin(), straw_values.end());
  const size_t middle = straw_values.size() / 2;
  const double median = (straw_values.size() % 2 == 0) ? ((straw_values[middle - 1] + straw_values[middle]) * 0.5) : straw_values[middle];
  const double threshold = median * median_factor;

  corners.append(0);

  int i = window_size;
  while (i < (point_count - window_size))
  {
    if (straw[i] < threshold)
    {
      int local_min_index = i;
      double local_min_value = straw[i];

      while (i < (point_count - window_size) && straw[i] < threshold)
      {
        if (straw[i] < local_min_value)
        {
          local_min_value = straw[i];
          local_min_index = i;
        }
        ++i;
      }

      if (local_min_index > 0 && local_min_index < (point_count - 1))
      {
        corners.append(local_min_index);
      }
    }
    else
    {
      ++i;
    }
  }

  corners.append(point_count - 1);
  std::sort(corners.begin(), corners.end());
  corners.erase(std::unique(corners.begin(), corners.end()), corners.end());

  const int max_iterations = point_count;
  for (int iteration = 0; iteration < max_iterations; ++iteration)
  {
    bool changed = false;
    QVector<int> refined;
    refined.reserve(corners.size() * 2);
    refined.append(corners.constFirst());

    for (int corner_idx = 0; corner_idx + 1 < corners.size(); ++corner_idx)
    {
      const int first = corners[corner_idx];
      const int last = corners[corner_idx + 1];

      if (last <= first + 1 || isLineSegmentApproximation(points, first, last, line_threshold))
      {
        refined.append(last);
        continue;
      }

      const int midpoint = (first + last) / 2;
      const int search_start = qMax(first + window_size, midpoint - window_size);
      const int search_end = qMin(last - window_size, midpoint + window_size);

      int best_corner = -1;
      double best_straw = std::numeric_limits<double>::infinity();

      for (int j = search_start; j <= search_end; ++j)
      {
        if (j <= first || j >= last)
        {
          continue;
        }
        if (straw[j] < best_straw)
        {
          best_straw = straw[j];
          best_corner = j;
        }
      }

      if (best_corner <= first || best_corner >= last)
      {
        best_corner = midpoint;
      }

      if (best_corner > first && best_corner < last)
      {
        refined.append(best_corner);
        changed = true;
      }
      refined.append(last);
    }

    std::sort(refined.begin(), refined.end());
    refined.erase(std::unique(refined.begin(), refined.end()), refined.end());
    corners = refined;

    if (!changed)
    {
      break;
    }
  }

  return corners;
}

bool PathSegmentation::isLineSegmentApproximation(const QVector<QPointF>& points, int first, int last, double threshold_ratio)
{
  if (first < 0 || last < 0 || first >= points.size() || last >= points.size() || first >= last)
  {
    return true;
  }

  double path_length = 0.0;
  for (int i = first; i < last; ++i)
  {
    path_length += QLineF(points[i], points[i + 1]).length();
  }

  if (path_length <= 1e-9)
  {
    return true;
  }

  const double chord_length = QLineF(points[first], points[last]).length();
  return (chord_length / path_length) >= threshold_ratio;
}

QVector<QPointF> PathSegmentation::simplifyWithLockedCorners(const QVector<QPointF>& points, const QVector<int>& locked_corner_indices, double epsilon_m)
{
  if (points.size() <= MIN_VALID_PATH_POINTS || epsilon_m <= 0.0)
  {
    return points;
  }

  QSet<int> locked_set;
  locked_set.insert(0);
  locked_set.insert(static_cast<int>(points.size()) - 1);

  for (const int index : locked_corner_indices)
  {
    if (index > 0 && index < (static_cast<int>(points.size()) - 1))
    {
      locked_set.insert(index);
    }
  }

  QVector<int> locked = locked_set.values();
  std::sort(locked.begin(), locked.end());

  QVector<int> merged_indices;
  for (int i = 0; i + 1 < locked.size(); ++i)
  {
    const int first = locked[i];
    const int last = locked[i + 1];
    if (last <= first)
    {
      continue;
    }

    QVector<int> segment_indices = collectRdpKeptIndices(points, first, last, epsilon_m);
    if (segment_indices.isEmpty())
    {
      continue;
    }

    if (!merged_indices.isEmpty() && merged_indices.constLast() == segment_indices.constFirst())
    {
      segment_indices.removeFirst();
    }

    merged_indices += segment_indices;
  }

  if (merged_indices.isEmpty())
  {
    return points;
  }

  QVector<QPointF> simplified;
  simplified.reserve(merged_indices.size());

  for (const int index : merged_indices)
  {
    if (index < 0 || index >= points.size())
    {
      continue;
    }

    const QPointF& point = points[index];
    if (simplified.isEmpty() || QLineF(simplified.constLast(), point).length() > 1e-9)
    {
      simplified.append(point);
    }
  }

  return simplified;
}

QVector<int> PathSegmentation::collectRdpKeptIndices(const QVector<QPointF>& points, int first, int last, double epsilon_m)
{
  QVector<int> kept_indices;

  if (points.isEmpty() || first < 0 || last < 0 || first >= points.size() || last >= points.size() || first > last)
  {
    return kept_indices;
  }

  std::function<void(int, int)> recurse = [&](int a, int b)
  {
    if (b <= a + 1)
    {
      kept_indices.append(a);
      kept_indices.append(b);
      return;
    }

    double max_distance = -1.0;
    int farthest_index = -1;

    for (int i = a + 1; i < b; ++i)
    {
      const double distance = pointToSegmentDistance(points[i], points[a], points[b]);
      if (distance > max_distance)
      {
        max_distance = distance;
        farthest_index = i;
      }
    }

    if (max_distance > epsilon_m && farthest_index > a && farthest_index < b)
    {
      recurse(a, farthest_index);
      recurse(farthest_index, b);
      return;
    }

    kept_indices.append(a);
    kept_indices.append(b);
  };

  recurse(first, last);
  std::sort(kept_indices.begin(), kept_indices.end());
  kept_indices.erase(std::unique(kept_indices.begin(), kept_indices.end()), kept_indices.end());
  return kept_indices;
}

double PathSegmentation::pointToSegmentDistance(const QPointF& point, const QPointF& seg_a, const QPointF& seg_b)
{
  const double vx = seg_b.x() - seg_a.x();
  const double vy = seg_b.y() - seg_a.y();
  const double length_sq = vx * vx + vy * vy;

  if (length_sq <= 1e-12)
  {
    return QLineF(point, seg_a).length();
  }

  const double wx = point.x() - seg_a.x();
  const double wy = point.y() - seg_a.y();
  const double t = qBound(0.0, (wx * vx + wy * vy) / length_sq, 1.0);
  const QPointF projection(seg_a.x() + t * vx, seg_a.y() + t * vy);
  return QLineF(point, projection).length();
}