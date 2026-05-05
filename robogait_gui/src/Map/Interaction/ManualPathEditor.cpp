#include <algorithm>
#include <functional>
#include <limits>
#include <vector>

#include <QDebug>
#include <QLineF>
#include <QVariantMap>

#include "Map/Interaction/ManualPathEditor.hpp"
#include "Map/MapVisualizationManager.hpp"

using namespace ROBOGait::map::interaction;

ManualPathEditor::ManualPathEditor(QObject* parent) :
    QObject(parent),
    map_visualization_manager_(nullptr),
    raw_path_points_(),
    segmented_path_points_(),
    is_segmented_(false),
    drawing_(false),
    stroke_blocked_by_outside_(false),
    min_point_distance_m_(0.02),
    segmentation_config_()
{
}

bool ManualPathEditor::hasPath() const { return getActivePathPoints().size() >= MIN_VALID_PATH_POINTS; }

bool ManualPathEditor::isSegmented() const { return is_segmented_ && !segmented_path_points_.isEmpty(); }

void ManualPathEditor::setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager) { map_visualization_manager_ = manager; }

bool ManualPathEditor::beginStroke()
{
  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[ManualPathEditor::beginStroke] MapVisualizationManager not set";
    return false;
  }

  // A path can only be drawn once per clear action
  if (hasPath())
  {
    return false;
  }

  stroke_blocked_by_outside_ = false;
  map_visualization_manager_->clearManualDrawPath();
  setDrawing(true);

  if (!appendRobotStartPoint())
  {
    qWarning() << "[ManualPathEditor::beginStroke] Robot pose unavailable, stroke canceled";
    setDrawing(false);
    return false;
  }

  syncPathToVisualization();
  return true;
}

void ManualPathEditor::beginStrokeFromScreen(double screen_x, double screen_y)
{
  if (!beginStroke())
  {
    return;
  }

  appendPointFromScreen(screen_x, screen_y);
}

void ManualPathEditor::appendPointFromScreen(double screen_x, double screen_y)
{
  if (!drawing_)
  {
    return;
  }

  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[ManualPathEditor::appendPointFromScreen] MapVisualizationManager not set";
    return;
  }

  QPointF map_point;
  const bool ok = map_visualization_manager_->screenToMap(QPointF(screen_x, screen_y), map_point);

  if (!ok)
  {
    qWarning() << "[ManualPathEditor::appendPointFromScreen] Failed to convert screen coordinates to map coordinates";
    return;
  }

  if (!map_visualization_manager_->isMapPointInside(map_point.x(), map_point.y()))
  {
    // If the pointer starts outside the map, ignore samples until it enters.
    // Once at least one drawable segment exists, leaving map blocks the rest of this drag.
    if (raw_path_points_.size() >= MIN_VALID_PATH_POINTS)
    {
      stroke_blocked_by_outside_ = true;
    }
    return;
  }

  if (stroke_blocked_by_outside_)
  {
    return;
  }

  if (!raw_path_points_.isEmpty() && QLineF(raw_path_points_.constLast(), map_point).length() < min_point_distance_m_)
  {
    return;
  }

  raw_path_points_.append(map_point);
  emit pathChanged();
  syncPathToVisualization();
}

void ManualPathEditor::endStroke()
{
  stroke_blocked_by_outside_ = false;
  setDrawing(false);
}

void ManualPathEditor::clear()
{
  if (map_visualization_manager_)
  {
    map_visualization_manager_->clearManualDrawPath();
    return;
  }

  clearCachedPath();
}

QVariantList ManualPathEditor::getPathPoints() const
{
  const QVector<QPointF>& points = getActivePathPoints();
  QVariantList points_list;
  points_list.reserve(points.size());

  for (const QPointF& point : points)
  {
    QVariantMap point_map;
    point_map["x"] = point.x();
    point_map["y"] = point.y();
    points_list.append(point_map);
  }

  return points_list;
}

void ManualPathEditor::setCachedPath(const QVariantList& points)
{
  QVector<QPointF> new_points;
  new_points.reserve(points.size());

  for (const auto& value : points)
  {
    if (!value.canConvert<QVariantMap>())
    {
      continue;
    }

    const QVariantMap map = value.toMap();
    bool ok_x = false;
    bool ok_y = false;
    const double x = map.value("x").toDouble(&ok_x);
    const double y = map.value("y").toDouble(&ok_y);

    if (!ok_x || !ok_y)
    {
      continue;
    }

    new_points.append(QPointF(x, y));
  }

  if (!arePointVectorsEqual(raw_path_points_, new_points) || !segmented_path_points_.isEmpty() || is_segmented_)
  {
    raw_path_points_ = new_points;
    segmented_path_points_.clear();
    setSegmentedState(false);
    emit pathChanged();
  }

  stroke_blocked_by_outside_ = false;
}

void ManualPathEditor::clearCachedPath()
{
  const bool had_path = hasPath();
  raw_path_points_.clear();
  segmented_path_points_.clear();
  setSegmentedState(false);
  stroke_blocked_by_outside_ = false;
  setDrawing(false);

  if (had_path)
  {
    emit pathChanged();
  }
}

bool ManualPathEditor::ensureMapVisualizationManager() const
{
  if (!map_visualization_manager_)
  {
    qCritical() << "[ManualPathEditor::ensureMapVisualizationManager] MapVisualizationManager pointer is null";
    return false;
  }

  return true;
}

void ManualPathEditor::syncPathToVisualization()
{
  if (!ensureMapVisualizationManager())
  {
    return;
  }

  map_visualization_manager_->setManualDrawPathPoints(getPathPoints());
}

void ManualPathEditor::setDrawing(bool drawing)
{
  if (drawing_ != drawing)
  {

    drawing_ = drawing;
    emit drawingChanged();
  }
}

void ManualPathEditor::setSegmentedState(bool segmented)
{
  if (is_segmented_ != segmented)
  {
    is_segmented_ = segmented;
    emit segmentedChanged();
  }
}

bool ManualPathEditor::segmentPath()
{
  if (!ensureMapVisualizationManager())
  {
    return false;
  }

  if (drawing_)
  {
    return false;
  }

  if (isSegmented())
  {
    return false;
  }

  if (raw_path_points_.size() < MIN_VALID_PATH_POINTS)
  {
    return false;
  }

  const QVector<QPointF> segmented_points = buildSegmentedPolyline(raw_path_points_);

  if (segmented_points.size() < MIN_VALID_PATH_POINTS)
  {
    return false;
  }

  const bool was_segmented = is_segmented_;
  const bool points_changed = !arePointVectorsEqual(segmented_path_points_, segmented_points);
  segmented_path_points_ = segmented_points;
  setSegmentedState(true);

  if (points_changed || !was_segmented)
  {
    emit pathChanged();
  }

  syncPathToVisualization();
  return true;
}

bool ManualPathEditor::arePointVectorsEqual(const QVector<QPointF>& lhs, const QVector<QPointF>& rhs)
{
  if (lhs.size() != rhs.size())
  {
    return false;
  }

  for (int i = 0; i < lhs.size(); ++i)
  {
    if (lhs[i] != rhs[i])
    {
      return false;
    }
  }

  return true;
}

const QVector<QPointF>& ManualPathEditor::getActivePathPoints() const
{
  if (is_segmented_ && !segmented_path_points_.isEmpty())
  {
    return segmented_path_points_;
  }

  return raw_path_points_;
}

double ManualPathEditor::pixelsToMetersDistance(double distance_px) const
{
  if (distance_px <= 0.0)
  {
    return 0.0;
  }

  if (map_visualization_manager_)
  {
    const double zoom = map_visualization_manager_->getZoomLevel();
    if (zoom > 1e-6)
    {
      return distance_px / zoom;
    }

    const double map_resolution = map_visualization_manager_->getMapResolution();
    if (map_resolution > 1e-9)
    {
      return distance_px * map_resolution;
    }
  }

  return distance_px * 0.01;
}

QVector<QPointF> ManualPathEditor::uniformResample(const QVector<QPointF>& points, double spacing_m) const
{
  if (points.size() <= 1 || spacing_m <= 1e-9)
  {
    return points;
  }

  QVector<QPointF> out;
  out.reserve(points.size());
  out.append(points.constFirst());

  double remaining_until_next_sample = spacing_m;

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

    while ((consumed + remaining_until_next_sample) <= (segment_length + 1e-9))
    {
      consumed += remaining_until_next_sample;
      const QPointF sample(segment_start.x() + direction.x() * consumed, segment_start.y() + direction.y() * consumed);

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

bool ManualPathEditor::isLineSegmentApproximation(const QVector<QPointF>& points, int first, int last, double threshold_ratio)
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

QVector<int> ManualPathEditor::detectShortStrawCorners(const QVector<QPointF>& points, int window, double median_factor, double line_threshold) const
{
  QVector<int> corners;
  const int point_count = points.size();

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

double ManualPathEditor::pointToSegmentDistance(const QPointF& point, const QPointF& seg_a, const QPointF& seg_b)
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

QVector<int> ManualPathEditor::collectRdpKeptIndices(const QVector<QPointF>& points, int first, int last, double epsilon_m) const
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

QVector<QPointF> ManualPathEditor::simplifyWithLockedCorners(const QVector<QPointF>& points, const QVector<int>& locked_corner_indices, double epsilon_m) const
{
  if (points.size() <= MIN_VALID_PATH_POINTS || epsilon_m <= 0.0)
  {
    return points;
  }

  QSet<int> locked_set;
  locked_set.insert(0);
  locked_set.insert(points.size() - 1);

  for (const int index : locked_corner_indices)
  {
    if (index > 0 && index < (points.size() - 1))
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

QVector<QPointF> ManualPathEditor::buildSegmentedPolyline(const QVector<QPointF>& points) const
{
  if (points.size() <= MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  const double spacing_m = qMax(pixelsToMetersDistance(segmentation_config_.resample_spacing_px), 1e-6);
  const double epsilon_m = qMax(pixelsToMetersDistance(segmentation_config_.rdp_epsilon_px), 1e-6);

  const QVector<QPointF> uniform_points = uniformResample(points, spacing_m);
  if (uniform_points.size() <= MIN_VALID_PATH_POINTS)
  {
    return uniform_points;
  }

  const QVector<int> corners = detectShortStrawCorners(uniform_points, segmentation_config_.short_straw_window, segmentation_config_.short_straw_median_factor,
                                                       segmentation_config_.short_straw_line_threshold);
  return simplifyWithLockedCorners(uniform_points, corners, epsilon_m);
}

bool ManualPathEditor::appendRobotStartPoint()
{
  if (!ensureMapVisualizationManager())
  {
    return false;
  }

  const QVariantMap pose = map_visualization_manager_->getRobotPose();
  const bool available = pose.value("available").toBool();
  if (!available)
  {
    return false;
  }

  bool ok_x = false;
  bool ok_y = false;
  const double x = pose.value("x").toDouble(&ok_x);
  const double y = pose.value("y").toDouble(&ok_y);
  if (!ok_x || !ok_y)
  {
    return false;
  }

  if (!map_visualization_manager_->isMapPointInside(x, y))
  {
    return false;
  }

  raw_path_points_.append(QPointF(x, y));
  emit pathChanged();
  return true;
}
