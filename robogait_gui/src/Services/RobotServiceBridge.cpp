#include <string>

#include <QMetaObject>
#include <QVariantMap>

#include <nav_msgs/msg/path.hpp>

#include "Services/RobotServiceBridge.hpp"

using namespace ROBOGait::qml::service;

RobotServiceBridge::RobotServiceBridge(QObject* parent) : QObject(parent), status_(RobotServiceBridge::Status::IDLE), active_command_key_(), poll_timer_(this)
{
  // Set up the polling timer
  poll_timer_.setInterval(POLL_INTERVAL_MS);
  poll_timer_.setSingleShot(false);

  // clang-format off
  connect(&poll_timer_,
          &QTimer::timeout,
          this,
          &RobotServiceBridge::onPoll);
  // clang-format on
  poll_timer_.start();

  syncFromClient();

  auto& client = ROBOGait::ros::service::RobotServiceClient::getInstance();

  // clang-format off
  client.setRequestCallback([this](bool success) {
    QMetaObject::invokeMethod(this,
                              [this, success]() { emit requestFinished(success); },
                              Qt::QueuedConnection);
  });
  // clang-format on

  // clang-format off
  client.setPathResultCallback([this](bool success, const nav_msgs::msg::Path& path) {
    QVariantList points;
    points.reserve(static_cast<int>(path.poses.size()));
    for (const auto& pose : path.poses)
    {
      QVariantMap point;
      point.insert("x", pose.pose.position.x);
      point.insert("y", pose.pose.position.y);
      points.append(point);
    }
    QMetaObject::invokeMethod(this,
                              [this, success, points]() { emit pathComputed(success, points); },
                              Qt::QueuedConnection);
  });
  // clang-format on

  // clang-format off
  client.setNavigationResultCallback([this](ROBOGait::ros::service::RobotServiceClient::NavigationResult result) {
    const NavigationResult mapped_result = toBridgeNavigationResult(result);
    const int result_code = static_cast<int>(mapped_result);
    QMetaObject::invokeMethod(this,
                              [this, result_code]() { emit navigationFinished(result_code); },
                              Qt::QueuedConnection);
  });
  // clang-format on
}

int RobotServiceBridge::getStatus() const { return status_; }

QString RobotServiceBridge::getActiveCommandKey() const { return active_command_key_; }

bool RobotServiceBridge::startMapping() { return ROBOGait::ros::service::RobotServiceClient::getInstance().startMapping(); }

bool RobotServiceBridge::stopMapping(bool save_map, const QString& map_name)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().stopMapping(save_map, map_name.trimmed().toStdString());
}

bool RobotServiceBridge::deleteMap(const QString& map_name)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().deleteMap(map_name.trimmed().toStdString());
}

bool RobotServiceBridge::requestMapData(const QString& map_name)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().requestMapData(map_name.trimmed().toStdString());
}

bool RobotServiceBridge::stopNavigation() { return ROBOGait::ros::service::RobotServiceClient::getInstance().stopNavigation(); }

bool RobotServiceBridge::startNavigation(const QString& map_name)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().startNavigation(map_name.trimmed().toStdString());
}

bool RobotServiceBridge::reinitializeGlobalLocalization() { return ROBOGait::ros::service::RobotServiceClient::getInstance().reinitializeGlobalLocalization(); }

bool RobotServiceBridge::computePathToPose(double x, double y, double theta)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().computePathToPose(x, y, theta);
}

bool RobotServiceBridge::computePathThroughPoses(const QVariantList& points)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().computePathThroughPoses(points);
}

bool RobotServiceBridge::navigateToPose(double x, double y, double theta)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().navigateToPose(x, y, theta);
}

bool RobotServiceBridge::navigateThroughPoses(const QVariantList& points)
{
  return ROBOGait::ros::service::RobotServiceClient::getInstance().navigateThroughPoses(points);
}

bool RobotServiceBridge::cancelNavigateToPose() { return ROBOGait::ros::service::RobotServiceClient::getInstance().cancelNavigateToPose(); }

void RobotServiceBridge::onPoll() { syncFromClient(); }

void RobotServiceBridge::syncFromClient()
{
  auto& client = ROBOGait::ros::service::RobotServiceClient::getInstance();

  ROBOGait::ros::service::RobotServiceClient::CommandStatus raw_status = ROBOGait::ros::service::RobotServiceClient::CommandStatus::IDLE;
  std::string raw_key;

  if (client.isInitialized())
  {
    raw_status = client.getActiveCommandStatus();
    raw_key = client.getActiveCommandKey();
  }

  const int new_status = static_cast<int>(toBridgeStatus(raw_status));
  const QString new_key = QString::fromStdString(raw_key);

  if (new_status != status_ || new_key != active_command_key_)
  {
    status_ = new_status;
    active_command_key_ = new_key;
    emit statusChanged();
  }
}

RobotServiceBridge::Status RobotServiceBridge::toBridgeStatus(ROBOGait::ros::service::RobotServiceClient::CommandStatus status)
{
  switch (status)
  {
    case ROBOGait::ros::service::RobotServiceClient::CommandStatus::IDLE:
      return RobotServiceBridge::Status::IDLE;
    case ROBOGait::ros::service::RobotServiceClient::CommandStatus::STARTING:
      return RobotServiceBridge::Status::STARTING;
    case ROBOGait::ros::service::RobotServiceClient::CommandStatus::RUNNING:
      return RobotServiceBridge::Status::RUNNING;
    case ROBOGait::ros::service::RobotServiceClient::CommandStatus::STOPPING:
      return RobotServiceBridge::Status::STOPPING;
    case ROBOGait::ros::service::RobotServiceClient::CommandStatus::STOPPED:
      return RobotServiceBridge::Status::STOPPED;
    case ROBOGait::ros::service::RobotServiceClient::CommandStatus::ERROR:
    default:
      return RobotServiceBridge::Status::ERROR;
  }
}

RobotServiceBridge::NavigationResult RobotServiceBridge::toBridgeNavigationResult(ROBOGait::ros::service::RobotServiceClient::NavigationResult result)
{
  switch (result)
  {
    case ROBOGait::ros::service::RobotServiceClient::NavigationResult::SUCCEEDED:
      return RobotServiceBridge::NavigationResult::NAV_SUCCEEDED;
    case ROBOGait::ros::service::RobotServiceClient::NavigationResult::CANCELED:
      return RobotServiceBridge::NavigationResult::NAV_CANCELED;
    case ROBOGait::ros::service::RobotServiceClient::NavigationResult::ABORTED:
      return RobotServiceBridge::NavigationResult::NAV_ABORTED;
    case ROBOGait::ros::service::RobotServiceClient::NavigationResult::UNKNOWN:
    default:
      return RobotServiceBridge::NavigationResult::NAV_UNKNOWN;
  }
}
