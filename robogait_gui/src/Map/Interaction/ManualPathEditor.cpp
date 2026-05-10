#include <QDebug>
#include <QLineF>
#include <QVariantMap>

#include "Map/Interaction/ManualPathEditor.hpp"
#include "Map/MapVisualizationManager.hpp"
#include "Map/Utils/PathSegmentation.hpp"

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
    segmentation_tuning_px_()
{
}

bool ManualPathEditor::hasPath() const { return getActivePathPoints().size() >= MIN_VALID_PATH_POINTS; }

bool ManualPathEditor::isSegmented() const { return is_segmented_ && !segmented_path_points_.isEmpty(); }

void ManualPathEditor::setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager) { map_visualization_manager_ = manager; }

void ManualPathEditor::setSegmentationTuningPx(double resample_spacing_px, double rdp_epsilon_px, int short_straw_window, double short_straw_median_factor,
                                               double short_straw_line_threshold)
{
  SegmentationTuningPx tuned = segmentation_tuning_px_;

  if (resample_spacing_px > 0.0)
  {
    tuned.resample_spacing_px = resample_spacing_px;
  }
  else
  {
    qWarning() << "[ManualPathEditor::setSegmentationTuningPx] Invalid resample_spacing_px, keeping previous value:" << tuned.resample_spacing_px;
  }

  if (rdp_epsilon_px > 0.0)
  {
    tuned.rdp_epsilon_px = rdp_epsilon_px;
  }
  else
  {
    qWarning() << "[ManualPathEditor::setSegmentationTuningPx] Invalid rdp_epsilon_px, keeping previous value:" << tuned.rdp_epsilon_px;
  }

  if (short_straw_window >= 1)
  {
    tuned.short_straw_window = short_straw_window;
  }
  else
  {
    qWarning() << "[ManualPathEditor::setSegmentationTuningPx] Invalid short_straw_window, keeping previous value:" << tuned.short_straw_window;
  }

  if (short_straw_median_factor > 0.0)
  {
    tuned.short_straw_median_factor = short_straw_median_factor;
  }
  else
  {
    qWarning() << "[ManualPathEditor::setSegmentationTuningPx] Invalid short_straw_median_factor, keeping previous value:"
               << tuned.short_straw_median_factor;
  }

  if (short_straw_line_threshold > 0.0 && short_straw_line_threshold <= 1.0)
  {
    tuned.short_straw_line_threshold = short_straw_line_threshold;
  }
  else
  {
    qWarning() << "[ManualPathEditor::setSegmentationTuningPx] Invalid short_straw_line_threshold, keeping previous value:"
               << tuned.short_straw_line_threshold;
  }

  segmentation_tuning_px_ = tuned;
}

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

QVector<QPointF> ManualPathEditor::buildSegmentedPolyline(const QVector<QPointF>& points) const
{
  if (points.size() <= MIN_VALID_PATH_POINTS)
  {
    return points;
  }

  ROBOGait::map::utils::PathSegmentation::Config config;
  config.resample_spacing_m = qMax(pixelsToMetersDistance(segmentation_tuning_px_.resample_spacing_px), 1e-6);
  config.rdp_epsilon_m = qMax(pixelsToMetersDistance(segmentation_tuning_px_.rdp_epsilon_px), 1e-6);
  config.short_straw_window = segmentation_tuning_px_.short_straw_window;
  config.short_straw_median_factor = segmentation_tuning_px_.short_straw_median_factor;
  config.short_straw_line_threshold = segmentation_tuning_px_.short_straw_line_threshold;
  return ROBOGait::map::utils::PathSegmentation::segmentPolyline(points, config);
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
