#include <cmath>

#include <QDebug>
#include <QLineF>
#include <QVariantMap>
#include <QtGlobal>

#include "Map/Interaction/Geometry/SplineFitter.hpp"
#include "Map/Interaction/Geometry/StrokeProcessor.hpp"
#include "Map/Interaction/SplinePathEditor.hpp"
#include "Map/MapVisualizationManager.hpp"

using namespace ROBOGait::map::interaction;

namespace
{
QVector<QPointF> removeCloseInteriorWaypoints(const QVector<QPointF>& points, double min_spacing_m)
{
  if (points.size() <= geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  QVector<QPointF> filtered;
  filtered.reserve(points.size());
  filtered.append(points.constFirst());

  for (int i = 1; i < points.size() - 1; ++i)
  {
    if (QLineF(filtered.constLast(), points[i]).length() >= min_spacing_m)
    {
      filtered.append(points[i]);
    }
  }

  if (QLineF(filtered.constLast(), points.constLast()).length() > 1e-9)
  {
    filtered.append(points.constLast());
  }

  if (filtered.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  return filtered;
}

QVector<QPointF> insertIntermediateWaypoints(const QVector<QPointF>& points, double max_spacing_m)
{
  if (points.size() < 2)
  {
    return points;
  }

  const double spacing = qMax(max_spacing_m, 1e-6);
  QVector<QPointF> expanded;
  expanded.reserve(points.size());
  expanded.append(points.constFirst());

  for (int i = 1; i < points.size(); ++i)
  {
    const QPointF start = points[i - 1];
    const QPointF end = points[i];
    const QLineF segment(start, end);
    const double segment_length = segment.length();

    if (segment_length <= 1e-9)
    {
      continue;
    }

    const int interior_samples = static_cast<int>(std::floor(segment_length / spacing));
    for (int sample = 1; sample <= interior_samples; ++sample)
    {
      const double distance = static_cast<double>(sample) * spacing;
      if (distance >= segment_length)
      {
        continue;
      }

      const double ratio = distance / segment_length;
      const QPointF interpolated(start.x() + (end.x() - start.x()) * ratio, start.y() + (end.y() - start.y()) * ratio);
      expanded.append(interpolated);
    }

    expanded.append(end);
  }

  return geometry::StrokeProcessor::removeDuplicatedPoints(expanded, 1e-9);
}
} // namespace

SplinePathEditor::SplinePathEditor(QObject* parent) :
    QObject(parent),
    map_visualization_manager_(nullptr),
    raw_path_points_(),
    smoothed_path_points_(),
    spline_model_(),
    is_smoothed_(false),
    is_edit_mode_(false),
    has_editable_path_(false),
    drawing_(false),
    stroke_blocked_by_outside_(false),
    min_point_distance_m_(0.02),
    smoothing_tuning_px_(),
    edit_reduction_tuning_px_(),
    planner_waypoint_tuning_()
{
}

bool SplinePathEditor::hasPath() const { return getActivePathPoints().size() >= geometry::StrokeProcessor::MIN_VALID_PATH_POINTS; }

bool SplinePathEditor::isSmoothed() const { return is_smoothed_ && !smoothed_path_points_.isEmpty(); }

bool SplinePathEditor::isEditMode() const { return is_edit_mode_; }

bool SplinePathEditor::hasEditablePath() const { return has_editable_path_; }

void SplinePathEditor::setEditMode(bool enabled) { setEditModeEnabled(enabled); }

void SplinePathEditor::setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager) { map_visualization_manager_ = manager; }

void SplinePathEditor::setSmoothingTuningPx(const SmoothingTuningPx& tuning)
{
  SmoothingTuningPx tuned = smoothing_tuning_px_;

  if (tuning.resample_spacing_px > 0.0)
  {
    tuned.resample_spacing_px = tuning.resample_spacing_px;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setSmoothingTuningPx] Invalid resample_spacing_px, keeping previous value:" << tuned.resample_spacing_px;
  }

  if (tuning.smoothing_window_radius >= 0)
  {
    tuned.smoothing_window_radius = tuning.smoothing_window_radius;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setSmoothingTuningPx] Invalid smoothing_window_radius, keeping previous value:" << tuned.smoothing_window_radius;
  }

  if (tuning.catmull_alpha >= 0.0 && tuning.catmull_alpha <= 1.0)
  {
    tuned.catmull_alpha = tuning.catmull_alpha;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setSmoothingTuningPx] Invalid catmull_alpha, keeping previous value:" << tuned.catmull_alpha;
  }

  if (tuning.sample_spacing_px > 0.0)
  {
    tuned.sample_spacing_px = tuning.sample_spacing_px;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setSmoothingTuningPx] Invalid sample_spacing_px, keeping previous value:" << tuned.sample_spacing_px;
  }

  smoothing_tuning_px_ = tuned;
}

void SplinePathEditor::setEditReductionTuningPx(const EditReductionTuningPx& tuning)
{
  EditReductionTuningPx tuned = edit_reduction_tuning_px_;

  if (tuning.simplify_tolerance_px > 0.0)
  {
    tuned.simplify_tolerance_px = tuning.simplify_tolerance_px;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setEditReductionTuningPx] Invalid simplify_tolerance_px, keeping previous value:" << tuned.simplify_tolerance_px;
  }

  if (tuning.max_anchor_points >= geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    tuned.max_anchor_points = tuning.max_anchor_points;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setEditReductionTuningPx] Invalid max_anchor_points, keeping previous value:" << tuned.max_anchor_points;
  }

  edit_reduction_tuning_px_ = tuned;
}

void SplinePathEditor::setPlannerWaypointTuning(const PlannerWaypointTuning& tuning)
{
  PlannerWaypointTuning tuned = planner_waypoint_tuning_;

  if (tuning.simplify_tolerance_m > 0.0)
  {
    tuned.simplify_tolerance_m = tuning.simplify_tolerance_m;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setPlannerWaypointTuning] Invalid simplify_tolerance_m, keeping previous value:" << tuned.simplify_tolerance_m;
  }

  if (tuning.min_waypoint_spacing_m > 0.0)
  {
    tuned.min_waypoint_spacing_m = tuning.min_waypoint_spacing_m;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setPlannerWaypointTuning] Invalid min_waypoint_spacing_m, keeping previous value:" << tuned.min_waypoint_spacing_m;
  }

  if (tuning.max_waypoint_spacing_m > 0.0)
  {
    tuned.max_waypoint_spacing_m = tuning.max_waypoint_spacing_m;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setPlannerWaypointTuning] Invalid max_waypoint_spacing_m, keeping previous value:" << tuned.max_waypoint_spacing_m;
  }

  if (tuned.max_waypoint_spacing_m < tuned.min_waypoint_spacing_m)
  {
    qWarning() << "[SplinePathEditor::setPlannerWaypointTuning] max_waypoint_spacing_m lower than min_waypoint_spacing_m, clamping to min value";
    tuned.max_waypoint_spacing_m = tuned.min_waypoint_spacing_m;
  }

  if (tuning.max_waypoints >= geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    tuned.max_waypoints = tuning.max_waypoints;
  }
  else
  {
    qWarning() << "[SplinePathEditor::setPlannerWaypointTuning] Invalid max_waypoints, keeping previous value:" << tuned.max_waypoints;
  }

  planner_waypoint_tuning_ = tuned;
}

bool SplinePathEditor::beginStroke()
{
  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[SplinePathEditor::beginStroke] MapVisualizationManager not set";
    return false;
  }

  if (hasPath())
  {
    return false;
  }

  stroke_blocked_by_outside_ = false;
  map_visualization_manager_->clearManualDrawPath();
  setDrawing(true);

  if (!appendRobotStartPoint())
  {
    qWarning() << "[SplinePathEditor::beginStroke] Robot pose unavailable, stroke canceled";
    setDrawing(false);
    return false;
  }

  syncPathToVisualization();
  return true;
}

void SplinePathEditor::clear()
{
  if (map_visualization_manager_)
  {
    map_visualization_manager_->clearManualDrawPath();
    return;
  }

  clearCachedPath();
}

void SplinePathEditor::clearCachedPath()
{
  const bool had_path = hasPath();
  raw_path_points_.clear();
  smoothed_path_points_.clear();
  spline_model_.clear();
  stroke_blocked_by_outside_ = false;
  setSmoothedState(false);
  setEditablePathState(false);
  setEditModeState(false);
  setDrawing(false);

  if (had_path)
  {
    emit pathChanged();
  }
}

void SplinePathEditor::beginStrokeFromScreen(double screen_x, double screen_y)
{
  if (!beginStroke())
  {
    return;
  }

  appendPointFromScreen(screen_x, screen_y);
}

void SplinePathEditor::appendPointFromScreen(double screen_x, double screen_y)
{
  if (!drawing_)
  {
    return;
  }

  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[SplinePathEditor::appendPointFromScreen] MapVisualizationManager not set";
    return;
  }

  QPointF map_point;
  const bool ok = map_visualization_manager_->screenToMap(QPointF(screen_x, screen_y), map_point);

  if (!ok)
  {
    qWarning() << "[SplinePathEditor::appendPointFromScreen] Failed to convert screen coordinates to map coordinates";
    return;
  }

  if (!map_visualization_manager_->isMapPointInside(map_point.x(), map_point.y()))
  {
    if (raw_path_points_.size() >= geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
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

void SplinePathEditor::endStroke()
{
  stroke_blocked_by_outside_ = false;
  setDrawing(false);
}

bool SplinePathEditor::smoothPath()
{
  if (!ensureMapVisualizationManager())
  {
    return false;
  }

  if (drawing_)
  {
    return false;
  }

  if (isSmoothed())
  {
    return false;
  }

  if (raw_path_points_.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    return false;
  }

  QVector<QPointF> processed = geometry::StrokeProcessor::removeDuplicatedPoints(raw_path_points_, 1e-6);
  const double resample_spacing_m = qMax(pixelsToMetersDistance(smoothing_tuning_px_.resample_spacing_px), 1e-6);
  processed = geometry::StrokeProcessor::resampleByArcLength(processed, resample_spacing_m);
  processed = geometry::StrokeProcessor::smoothMovingAverage(processed, smoothing_tuning_px_.smoothing_window_radius);
  const double simplify_tolerance_m = qMax(pixelsToMetersDistance(edit_reduction_tuning_px_.simplify_tolerance_px), 1e-6);
  processed = geometry::StrokeProcessor::simplifyDouglasPeucker(processed, simplify_tolerance_m);
  processed = geometry::StrokeProcessor::limitPointCount(processed, edit_reduction_tuning_px_.max_anchor_points);

  if (processed.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    return false;
  }

  const QVector<geometry::CubicBezierSegment> segments = geometry::SplineFitter::fitCentripetalCatmullRom(processed, smoothing_tuning_px_.catmull_alpha);

  if (segments.isEmpty() || !spline_model_.setFromSegments(segments))
  {
    return false;
  }

  const double sample_spacing_m = qMax(pixelsToMetersDistance(smoothing_tuning_px_.sample_spacing_px), 1e-6);
  smoothed_path_points_ = spline_model_.sampleByDistance(sample_spacing_m);

  if (smoothed_path_points_.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    spline_model_.clear();
    smoothed_path_points_.clear();
    return false;
  }

  setSmoothedState(true);
  setEditablePathState(true);
  setEditModeState(false);
  emit pathChanged();
  syncPathToVisualization();
  return true;
}

bool SplinePathEditor::setEditModeEnabled(bool enabled)
{
  if (enabled && !isSmoothed())
  {
    return false;
  }

  setEditModeState(enabled);
  return true;
}

bool SplinePathEditor::moveControlPoint(int type, int index, double map_x, double map_y)
{
  if (!isSmoothed())
  {
    return false;
  }

  if (!std::isfinite(map_x) || !std::isfinite(map_y))
  {
    return false;
  }

  const geometry::ControlPointType point_type = static_cast<geometry::ControlPointType>(type);
  const bool updated = spline_model_.setControlPoint(point_type, index, QPointF(map_x, map_y));

  if (!updated)
  {
    return false;
  }

  const double sample_spacing_m = qMax(pixelsToMetersDistance(smoothing_tuning_px_.sample_spacing_px), 1e-6);
  smoothed_path_points_ = spline_model_.sampleByDistance(sample_spacing_m);
  setEditablePathState(smoothed_path_points_.size() >= geometry::StrokeProcessor::MIN_VALID_PATH_POINTS);
  emit pathChanged();
  syncPathToVisualization();
  return true;
}

QVariantList SplinePathEditor::getControlPoints() const
{
  QVariantList points;

  if (!isSmoothed())
  {
    return points;
  }

  const QVector<geometry::BezierSplineModel::ControlPointRef> controls = spline_model_.getControlPoints();
  points.reserve(controls.size());

  for (const geometry::BezierSplineModel::ControlPointRef& control : controls)
  {
    if (control.type != geometry::ControlPointType::Anchor)
    {
      continue;
    }

    QVariantMap point_map;
    point_map["type"] = static_cast<int>(control.type);
    point_map["index"] = control.index;
    point_map["x"] = control.position.x();
    point_map["y"] = control.position.y();
    points.append(point_map);
  }

  return points;
}

QVariantList SplinePathEditor::getPathPoints() const { return toVariantList(getActivePathPoints()); }

QVariantList SplinePathEditor::getPathPointsForCompute() const
{
  if (!hasEditablePath())
  {
    return QVariantList();
  }

  return toVariantList(smoothed_path_points_);
}

QVariantList SplinePathEditor::getPlannerWaypointsForCompute() const
{
  if (!hasEditablePath())
  {
    return QVariantList();
  }

  QVector<QPointF> waypoints = geometry::StrokeProcessor::removeDuplicatedPoints(smoothed_path_points_, 1e-6);
  waypoints = geometry::StrokeProcessor::simplifyDouglasPeucker(waypoints, planner_waypoint_tuning_.simplify_tolerance_m);
  waypoints = removeCloseInteriorWaypoints(waypoints, planner_waypoint_tuning_.min_waypoint_spacing_m);
  waypoints = insertIntermediateWaypoints(waypoints, planner_waypoint_tuning_.max_waypoint_spacing_m);
  waypoints = geometry::StrokeProcessor::limitPointCount(waypoints, planner_waypoint_tuning_.max_waypoints);

  if (waypoints.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    return QVariantList();
  }

  return toVariantList(waypoints);
}

bool SplinePathEditor::ensureMapVisualizationManager() const
{
  if (!map_visualization_manager_)
  {
    qCritical() << "[SplinePathEditor::ensureMapVisualizationManager] MapVisualizationManager pointer is null";
    return false;
  }

  return true;
}

void SplinePathEditor::syncPathToVisualization()
{
  if (!ensureMapVisualizationManager())
  {
    return;
  }

  map_visualization_manager_->setManualDrawPathPoints(getPathPoints());
}

void SplinePathEditor::setDrawing(bool drawing)
{
  if (drawing_ != drawing)
  {
    drawing_ = drawing;
    emit drawingChanged();
  }
}

void SplinePathEditor::setSmoothedState(bool smoothed)
{
  if (is_smoothed_ != smoothed)
  {
    is_smoothed_ = smoothed;
    emit smoothedChanged();
  }
}

void SplinePathEditor::setEditablePathState(bool editable)
{
  if (has_editable_path_ != editable)
  {
    has_editable_path_ = editable;
    emit editablePathChanged();
  }
}

void SplinePathEditor::setEditModeState(bool enabled)
{
  if (is_edit_mode_ != enabled)
  {
    is_edit_mode_ = enabled;
    emit editModeChanged();
  }
}

bool SplinePathEditor::appendRobotStartPoint()
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

const QVector<QPointF>& SplinePathEditor::getActivePathPoints() const
{
  if (is_smoothed_ && !smoothed_path_points_.isEmpty())
  {
    return smoothed_path_points_;
  }

  return raw_path_points_;
}

double SplinePathEditor::pixelsToMetersDistance(double distance_px) const
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

QVariantList SplinePathEditor::toVariantList(const QVector<QPointF>& points)
{
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
