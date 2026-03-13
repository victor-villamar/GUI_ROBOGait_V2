#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "CommandExecutorClient.hpp"

namespace ROBOGait
{
namespace qml
{
namespace executor
{
/**
 * @brief Bridge class to expose the CommandExecutorClient to QML
 */
class CommandExecutorBridge : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Enum to represent the status of the command executor
   */
  enum Status
  {
    IDLE = 0, /**< The command executor is idle */
    STARTING, /**< The command executor is starting */
    RUNNING,  /**< The command executor is running */
    STOPPING, /**< The command executor is stopping */
    STOPPED,  /**< The command executor has stopped */
    ERROR     /**< There was an error with the command executor */
  };
  Q_ENUM(Status)

  // clang-format off
  Q_PROPERTY(int status
             READ getStatus
             NOTIFY statusChanged)
  Q_PROPERTY(QString activeCommandKey
             READ getActiveCommandKey
             NOTIFY statusChanged)
  // clang-format on

  /**
   * @brief Constructor of the CommandExecutorBridge class
   */
  explicit CommandExecutorBridge(QObject* parent = nullptr);

  /**
   * @brief Get the status of the command executor
   *
   * @return The status of the command executor
   */
  int getStatus() const;

  /**
   * @brief Get the active command key
   *
   * @return The active command key
   */
  QString getActiveCommandKey() const;

  /**
   * @brief Start the mapping process
   *
   * @return true if the mapping process was started successfully, false otherwise
   */
  Q_INVOKABLE bool startMapping();

  /**
   * @brief Stop the mapping process
   *
   * @param save_map Whether to save the map
   * @param map_name The name of the map to save
   *
   * @return true if the mapping process was stopped successfully, false otherwise
   */
  Q_INVOKABLE bool stopMapping(bool save_map, const QString& map_name);

  /**
   * @brief Delete a map
   *
   * @param map_name The name of the map to delete
   *
   * @return true if the map was deleted successfully, false otherwise
   */
  Q_INVOKABLE bool deleteMap(const QString& map_name);

signals:
  void statusChanged(); // Emitted when the status changes

private slots:
  void onPoll(); // Polling slot to update the status

private:
  /**
   * @brief Synchronize the status from the CommandExecutorClient
   */
  void syncFromClient();

  /**
   * @brief Convert the CommandStatus from the CommandExecutorClient to the Bridge Status
   *
   * @param status The CommandStatus from the CommandExecutorClient
   *
   * @return The corresponding Bridge Status
   */
  Status toBridgeStatus(ROBOGait::ros::executor::CommandExecutorClient::CommandStatus status);

  int status_;                 /**< The current status of the command executor */
  QString active_command_key_; /**< The active command key */
  QTimer poll_timer_;          /**< The timer used for polling the command executor status */

  static constexpr int POLL_INTERVAL_MS = 200; /**< The polling interval in milliseconds */
};
} // namespace executor
} // namespace qml
} // namespace ROBOGait
