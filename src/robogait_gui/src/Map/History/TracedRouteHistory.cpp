#include <cmath>

#include <QVariant>

#include "Map/History/TracedRouteHistory.hpp"

using namespace ROBOGait::map::history;

TracedRouteHistory::TracedRouteHistory(QObject* parent) : QObject(parent), routes_(), next_route_id_(0), goal_count_(0), path_count_(0), selected_route_id_(-1)
{
}

QVariantList TracedRouteHistory::getRoutes() const { return routes_; }

int TracedRouteHistory::getSelectedRouteId() const { return selected_route_id_; }

bool TracedRouteHistory::isEmpty() const { return routes_.isEmpty(); }

QVariantMap TracedRouteHistory::addGoalRoute(double x, double y, double theta, const QVariantMap& robot_pose)
{
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(theta))
  {
    return QVariantMap();
  }

  ++next_route_id_;
  ++goal_count_;

  QVariantMap goal;
  goal.insert("x", x);
  goal.insert("y", y);
  goal.insert("theta", theta);

  QVariantMap route;
  route.insert("id", next_route_id_);
  route.insert("type", "goal");
  route.insert("name", tr("Objetivo %1").arg(goal_count_));
  route.insert("goal", goal);
  route.insert("robotPose", normalizedRobotPose(robot_pose));

  prependRoute(route);
  selectRoute(next_route_id_);

  return route;
}

QVariantMap TracedRouteHistory::addPathRoute(const QVariantList& points, double terminal_theta, const QVariantMap& robot_pose)
{
  const QVariantList cloned_points = clonePathPoints(points);
  if (cloned_points.size() < 2)
  {
    return QVariantMap();
  }

  ++next_route_id_;
  ++path_count_;

  QVariantMap route;
  route.insert("id", next_route_id_);
  route.insert("type", "path");
  route.insert("name", tr("Ruta %1").arg(path_count_));
  route.insert("points", cloned_points);
  route.insert("terminalTheta", std::isfinite(terminal_theta) ? terminal_theta : 0.0);
  route.insert("robotPose", normalizedRobotPose(robot_pose));

  prependRoute(route);
  selectRoute(next_route_id_);

  return route;
}

void TracedRouteHistory::selectRoute(int route_id)
{
  if (selected_route_id_ == route_id)
  {
    return;
  }

  selected_route_id_ = route_id;
  emit selectedRouteIdChanged();
}

void TracedRouteHistory::clear()
{
  if (routes_.isEmpty() && next_route_id_ == 0 && goal_count_ == 0 && path_count_ == 0 && selected_route_id_ == -1)
  {
    return;
  }

  routes_.clear();
  next_route_id_ = 0;
  goal_count_ = 0;
  path_count_ = 0;

  const bool selected_route_changed = selected_route_id_ != -1;
  selected_route_id_ = -1;

  emit routesChanged();
  if (selected_route_changed)
  {
    emit selectedRouteIdChanged();
  }
}

QVariantList TracedRouteHistory::clonePathPoints(const QVariantList& points) const
{
  QVariantList cloned_points;
  cloned_points.reserve(points.size());

  for (const QVariant& point_variant : points)
  {
    const QVariantMap point_map = point_variant.toMap();
    bool ok_x = false;
    bool ok_y = false;
    const double x = point_map.value("x").toDouble(&ok_x);
    const double y = point_map.value("y").toDouble(&ok_y);

    if (!ok_x || !ok_y || !std::isfinite(x) || !std::isfinite(y))
    {
      continue;
    }

    QVariantMap point;
    point.insert("x", x);
    point.insert("y", y);

    bool ok_theta = false;
    const double theta = point_map.value("theta").toDouble(&ok_theta);
    if (ok_theta && std::isfinite(theta))
    {
      point.insert("theta", theta);
    }

    cloned_points.append(point);
  }

  return cloned_points;
}

QVariantMap TracedRouteHistory::normalizedRobotPose(const QVariantMap& robot_pose) const
{
  QVariantMap normalized_pose;

  bool ok_x = false;
  bool ok_y = false;
  bool ok_theta = false;
  const double x = robot_pose.value("x").toDouble(&ok_x);
  const double y = robot_pose.value("y").toDouble(&ok_y);
  const double theta = robot_pose.value("theta").toDouble(&ok_theta);

  normalized_pose.insert("x", ok_x && std::isfinite(x) ? x : 0.0);
  normalized_pose.insert("y", ok_y && std::isfinite(y) ? y : 0.0);
  normalized_pose.insert("theta", ok_theta && std::isfinite(theta) ? theta : 0.0);

  return normalized_pose;
}

void TracedRouteHistory::prependRoute(const QVariantMap& route)
{
  routes_.prepend(route);
  emit routesChanged();
}
