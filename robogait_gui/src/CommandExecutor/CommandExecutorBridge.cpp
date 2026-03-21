#include <string>

#include "CommandExecutor/CommandExecutorBridge.hpp"

using namespace ROBOGait::qml::executor;

CommandExecutorBridge::CommandExecutorBridge(QObject* parent) :
    QObject(parent), status_(CommandExecutorBridge::Status::IDLE), active_command_key_(), poll_timer_(this)
{
  // Set up the polling timer
  poll_timer_.setInterval(POLL_INTERVAL_MS);
  poll_timer_.setSingleShot(false);

  // clang-format off
  connect(&poll_timer_,
          &QTimer::timeout,
          this,
          &CommandExecutorBridge::onPoll);
  // clang-format on
  poll_timer_.start();

  syncFromClient();
}

int CommandExecutorBridge::getStatus() const { return status_; }

QString CommandExecutorBridge::getActiveCommandKey() const { return active_command_key_; }

bool CommandExecutorBridge::startMapping() { return ROBOGait::ros::executor::CommandExecutorClient::getInstance().startMapping(); }

bool CommandExecutorBridge::stopMapping(bool save_map, const QString& map_name)
{
  return ROBOGait::ros::executor::CommandExecutorClient::getInstance().stopMapping(save_map, map_name.trimmed().toStdString());
}

bool CommandExecutorBridge::deleteMap(const QString& map_name)
{
  return ROBOGait::ros::executor::CommandExecutorClient::getInstance().deleteMap(map_name.trimmed().toStdString());
}

bool CommandExecutorBridge::requestMapData(const QString& map_name)
{
  return ROBOGait::ros::executor::CommandExecutorClient::getInstance().requestMapData(map_name.trimmed().toStdString());
}


bool CommandExecutorBridge::stopNavigation()
{
  return ROBOGait::ros::executor::CommandExecutorClient::getInstance().stopNavigation();
}

bool CommandExecutorBridge::startNavigation(const QString& map_name)
{
  return ROBOGait::ros::executor::CommandExecutorClient::getInstance().startNavigation(map_name.trimmed().toStdString());
}

void CommandExecutorBridge::onPoll() { syncFromClient(); }

void CommandExecutorBridge::syncFromClient()
{
  auto& client = ROBOGait::ros::executor::CommandExecutorClient::getInstance();

  ROBOGait::ros::executor::CommandExecutorClient::CommandStatus raw_status = ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::IDLE;
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

CommandExecutorBridge::Status CommandExecutorBridge::toBridgeStatus(ROBOGait::ros::executor::CommandExecutorClient::CommandStatus status)
{
  switch (status)
  {
    case ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::IDLE:
      return CommandExecutorBridge::Status::IDLE;
    case ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::STARTING:
      return CommandExecutorBridge::Status::STARTING;
    case ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::RUNNING:
      return CommandExecutorBridge::Status::RUNNING;
    case ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::STOPPING:
      return CommandExecutorBridge::Status::STOPPING;
    case ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::STOPPED:
      return CommandExecutorBridge::Status::STOPPED;
    case ROBOGait::ros::executor::CommandExecutorClient::CommandStatus::ERROR:
    default:
      return CommandExecutorBridge::Status::ERROR;
  }
}
