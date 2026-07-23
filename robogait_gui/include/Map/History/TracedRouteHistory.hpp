#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace ROBOGait
{
namespace map
{
namespace history
{

/**
 * @brief Session-local storage for routes traced during a test flow
 */
class TracedRouteHistory : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QVariantList routes
             READ getRoutes
             NOTIFY routesChanged)

  Q_PROPERTY(int selectedRouteId
             READ getSelectedRouteId
             NOTIFY selectedRouteIdChanged)

  Q_PROPERTY(bool isEmpty
             READ isEmpty
             NOTIFY routesChanged)
  // clang-format on

  explicit TracedRouteHistory(QObject* parent = nullptr);

  QVariantList getRoutes() const;

  int getSelectedRouteId() const;

  bool isEmpty() const;

  Q_INVOKABLE QVariantMap addGoalRoute(double x, double y, double theta, const QVariantMap& robot_pose);

  Q_INVOKABLE QVariantMap addPathRoute(const QVariantList& points, double terminal_theta, const QVariantMap& robot_pose);

  Q_INVOKABLE void selectRoute(int route_id);

  Q_INVOKABLE void clear();

signals:
  void routesChanged();          // Emitted when routes are added or cleared
  void selectedRouteIdChanged(); // Emitted when the selected route changes

private:
  QVariantList clonePathPoints(const QVariantList& points) const;

  QVariantMap normalizedRobotPose(const QVariantMap& robot_pose) const;

  void prependRoute(const QVariantMap& route);

  QVariantList routes_;   /**< List of all routes traced during the session */
  int next_route_id_;     /**< Next unique route ID to assign */
  int goal_count_;        /**< Count of goal routes added during the session */
  int path_count_;        /**< Count of path routes added during the session */
  int selected_route_id_; /**< ID of the currently selected route */
};

} // namespace history
} // namespace map
} // namespace ROBOGait
