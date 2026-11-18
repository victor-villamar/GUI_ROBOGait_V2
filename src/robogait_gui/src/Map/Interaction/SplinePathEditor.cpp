#include <cmath>

#include <QDebug>
#include <QLineF>
#include <QVariantMap>
#include <QtGlobal>
#include <qglobal.h>

#ifdef ROBOGAIT_ENABLE_PATH_VALIDATION_EXPORT
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QSaveFile>
#include <QTextStream>
#endif

#include "Map/Interaction/Geometry/SplineFitter.hpp"
#include "Map/Interaction/Geometry/StrokeProcessor.hpp"
#include "Map/Interaction/SplinePathEditor.hpp"
#include "Map/MapVisualizationManager.hpp"

using namespace ROBOGait::map::interaction;

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
    edit_reduction_tuning_px_()
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

bool SplinePathEditor::loadPathPoints(const QVariantList& points)
{
  if (!ensureMapVisualizationManager())
  {
    return false;
  }

  QVector<QPointF> loaded_points;
  loaded_points.reserve(points.size());

  for (const QVariant& value : points)
  {
    if (!value.canConvert<QVariantMap>())
    {
      continue;
    }

    const QVariantMap point_map = value.toMap();
    bool ok_x = false;
    bool ok_y = false;
    const double x = point_map.value("x").toDouble(&ok_x);
    const double y = point_map.value("y").toDouble(&ok_y);

    if (!ok_x || !ok_y || !std::isfinite(x) || !std::isfinite(y))
    {
      continue;
    }

    loaded_points.append(QPointF(x, y));
  }

  loaded_points = geometry::StrokeProcessor::removeDuplicatedPoints(loaded_points, 1e-6);

  if (loaded_points.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    qWarning() << "[SplinePathEditor::loadPathPoints] Not enough valid points to load path";
    return false;
  }

  QVector<QPointF> editable_points = loaded_points;
  const double simplify_tolerance_m = qMax(pixelsToMetersDistance(edit_reduction_tuning_px_.simplify_tolerance_px), 1e-6);
  editable_points = geometry::StrokeProcessor::simplifyDouglasPeucker(editable_points, simplify_tolerance_m);
  editable_points = geometry::StrokeProcessor::limitPointCount(editable_points, edit_reduction_tuning_px_.max_anchor_points);

  if (editable_points.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    qWarning() << "[SplinePathEditor::loadPathPoints] Not enough editable points to rebuild spline";
    return false;
  }

  const QVector<geometry::CubicBezierSegment> segments = geometry::SplineFitter::fitCentripetalCatmullRom(editable_points, smoothing_tuning_px_.catmull_alpha);

  if (segments.isEmpty() || !spline_model_.setFromSegments(segments))
  {
    qWarning() << "[SplinePathEditor::loadPathPoints] Failed to rebuild editable spline";
    return false;
  }

  const double sample_spacing_m = qMax(pixelsToMetersDistance(smoothing_tuning_px_.sample_spacing_px), 1e-6);
  const QVector<QPointF> sampled_points = spline_model_.sampleByDistance(sample_spacing_m);

  if (sampled_points.size() < geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    qWarning() << "[SplinePathEditor::loadPathPoints] Rebuilt spline produced an invalid path";
    return false;
  }

  raw_path_points_ = loaded_points;
  smoothed_path_points_ = sampled_points;
  stroke_blocked_by_outside_ = false;
  setDrawing(false);
  setSmoothedState(true);
  setEditablePathState(true);
  setEditModeState(false);
  emit pathChanged();
  syncPathToVisualization();
  return true;
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

#ifdef ROBOGAIT_ENABLE_PATH_VALIDATION_EXPORT
  exportValidationTrace();
#endif

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

#ifdef ROBOGAIT_ENABLE_PATH_VALIDATION_EXPORT
void SplinePathEditor::exportValidationTrace() const
{
  const QString output_directory = QDir::home().filePath(".local/robogait/path_validation/raw");
  QDir directory;

  if (!directory.mkpath(output_directory))
  {
    qCritical() << "[SplinePathEditor::exportValidationTrace] Failed to create output directory:" << output_directory;
    return;
  }

  directory.setPath(output_directory);
  const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
  QString capture_name = QString("path_%1").arg(timestamp);
  int suffix = 1;

  while (QFileInfo::exists(directory.filePath(capture_name + "_raw.csv")) || QFileInfo::exists(directory.filePath(capture_name + "_metadata.csv")))
  {
    capture_name = QString("path_%1_%2").arg(timestamp).arg(suffix++);
  }

  const QString raw_path = directory.filePath(capture_name + "_raw.csv");
  const QString metadata_path = directory.filePath(capture_name + "_metadata.csv");
  QSaveFile raw_file(raw_path);
  QSaveFile metadata_file(metadata_path);

  if (!raw_file.open(QIODevice::WriteOnly | QIODevice::Text) || !metadata_file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    qCritical() << "[SplinePathEditor::exportValidationTrace] Failed to open validation files for writing in:" << output_directory;
    raw_file.cancelWriting();
    metadata_file.cancelWriting();
    return;
  }

  QTextStream raw_stream(&raw_file);
  raw_stream.setLocale(QLocale::c());
  raw_stream.setRealNumberNotation(QTextStream::FixedNotation);
  raw_stream.setRealNumberPrecision(10);
  raw_stream << "index,x_m,y_m\n";
  for (int index = 0; index < raw_path_points_.size(); ++index)
  {
    raw_stream << index << ',' << raw_path_points_.at(index).x() << ',' << raw_path_points_.at(index).y() << '\n';
  }

  const double zoom_level = map_visualization_manager_->getZoomLevel();
  const double map_resolution = map_visualization_manager_->getMapResolution();
  const double resample_spacing_m = pixelsToMetersDistance(smoothing_tuning_px_.resample_spacing_px);
  const double simplify_tolerance_m = pixelsToMetersDistance(edit_reduction_tuning_px_.simplify_tolerance_px);
  const double sample_spacing_m = pixelsToMetersDistance(smoothing_tuning_px_.sample_spacing_px);

  QTextStream metadata_stream(&metadata_file);
  metadata_stream.setLocale(QLocale::c());
  metadata_stream.setRealNumberNotation(QTextStream::FixedNotation);
  metadata_stream.setRealNumberPrecision(10);
  metadata_stream << "parameter,value\n"
                  << "zoom_level," << zoom_level << '\n'
                  << "map_resolution_m_per_px," << map_resolution << '\n'
                  << "resample_spacing_px," << smoothing_tuning_px_.resample_spacing_px << '\n'
                  << "resample_spacing_m," << resample_spacing_m << '\n'
                  << "smoothing_window_radius," << smoothing_tuning_px_.smoothing_window_radius << '\n'
                  << "simplify_tolerance_px," << edit_reduction_tuning_px_.simplify_tolerance_px << '\n'
                  << "simplify_tolerance_m," << simplify_tolerance_m << '\n'
                  << "max_anchor_points," << edit_reduction_tuning_px_.max_anchor_points << '\n'
                  << "catmull_alpha," << smoothing_tuning_px_.catmull_alpha << '\n'
                  << "sample_spacing_px," << smoothing_tuning_px_.sample_spacing_px << '\n'
                  << "sample_spacing_m," << sample_spacing_m << '\n'
                  << "raw_point_count," << raw_path_points_.size() << '\n';

  if (!raw_file.commit() || !metadata_file.commit())
  {
    qCritical() << "[SplinePathEditor::exportValidationTrace] Failed to save validation files in:" << output_directory;
    return;
  }

  qInfo() << "[SplinePathEditor::exportValidationTrace] Trace exported successfully to:" << raw_path << "and" << metadata_path;
}
#endif

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
