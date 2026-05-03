#include <QDebug>
#include <QLineF>
#include <QVariantMap>

#include "Map/Interaction/ManualPathEditor.hpp"
#include "Map/MapVisualizationManager.hpp"

using namespace ROBOGait::map::interaction;

ManualPathEditor::ManualPathEditor(QObject* parent) :
    QObject(parent), map_visualization_manager_(nullptr), points_(), drawing_(false), stroke_blocked_by_outside_(false), min_point_distance_m_(0.02)
{
}

bool ManualPathEditor::hasPath() const { return !points_.isEmpty(); }

void ManualPathEditor::setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager) { map_visualization_manager_ = manager; }

bool ManualPathEditor::beginStroke()
{
  if (!ensureMapVisualizationManager())
  {
    qCritical() << "[ManualPathEditor::beginStroke] MapVisualizationManager not set";
    return false;
  }

  // A path can only be drawn once per clear action
  if (!points_.isEmpty())
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
    stroke_blocked_by_outside_ = true;
    return;
  }

  if (stroke_blocked_by_outside_)
  {
    return;
  }

  if (!points_.isEmpty() && QLineF(points_.constLast(), map_point).length() < min_point_distance_m_)
  {
    return;
  }

  points_.append(map_point);
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
  QVariantList points_list;
  points_list.reserve(points_.size());

  for (const QPointF& point : points_)
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

  if (!arePointVectorsEqual(points_, new_points))
  {
    points_ = new_points;
    emit pathChanged();
  }

  stroke_blocked_by_outside_ = false;
}

void ManualPathEditor::clearCachedPath()
{
  const bool had_path = !points_.isEmpty();
  points_.clear();
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

  points_.append(QPointF(x, y));
  emit pathChanged();
  return true;
}
