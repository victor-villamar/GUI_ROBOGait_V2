#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>

#include "Services/RobotServiceClient.hpp"

namespace ROBOGait
{
namespace qml
{
namespace service
{
/**
 * @brief Bridge class to expose the RobotServiceClient to QML
 */
class RobotServiceBridge : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Enum to represent the status of the commands executed by the RobotServiceClient
   */
  enum Status
  {
    IDLE = 0, /**< The command is idle */
    STARTING, /**< The command is starting */
    RUNNING,  /**< The command is running */
    STOPPING, /**< The command is stopping */
    STOPPED,  /**< The command has stopped */
    ERROR     /**< There was an error with the command */
  };
  Q_ENUM(Status)

  /**
   * @brief Enum to represent navigation action final result
   */
  enum NavigationResult
  {
    NAV_UNKNOWN = 0,   /**< Unknown result */
    NAV_SUCCEEDED = 1, /**< Goal reached */
    NAV_CANCELED = 2,  /**< Goal canceled */
    NAV_ABORTED = 3    /**< Goal aborted */
  };
  Q_ENUM(NavigationResult)

  // clang-format off
  Q_PROPERTY(int status
             READ getStatus
             NOTIFY statusChanged)

  Q_PROPERTY(QString activeCommandKey
             READ getActiveCommandKey
             NOTIFY statusChanged)
  // clang-format on

  /**
   * @brief Constructor of the RobotServiceBridge class
   */
  explicit RobotServiceBridge(QObject* parent = nullptr);

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

  /**
   * @brief Request the map data for a given map name
   *
   * @param map_name The name of the map to request data for
   *
   * @return true if the map data was requested successfully, false otherwise
   */
  Q_INVOKABLE bool requestMapData(const QString& map_name);

  /**
   * @brief Start the navigation process
   *
   * @param map_name The name of the map to use for navigation
   *
   * @return true if the navigation process was started successfully, false otherwise
   */
  Q_INVOKABLE bool startNavigation(const QString& map_name);

  /**
   * @brief Stop the navigation process
   *
   * @return true if the navigation process was stopped successfully, false otherwise
   */
  Q_INVOKABLE bool stopNavigation();

  /** @brief Reinitialize the global localization
   *
   * @return true if the global localization was reinitialized successfully, false otherwise
   */
  Q_INVOKABLE bool reinitializeGlobalLocalization();

  /**
   * @brief Compute path to a goal pose
   *
   * @param x Goal x in map frame
   * @param y Goal y in map frame
   * @param theta Goal yaw in radians
   *
   * @return true if the request was sent, false otherwise
   */
  Q_INVOKABLE bool computePathToPose(double x, double y, double theta);

  /**
   * @brief Compute path through multiple poses
   *
   * @param points Goal poses in map frame as list of maps with keys x, y, and optional theta
   *
   * @return true if the request was sent, false otherwise
   */
  Q_INVOKABLE bool computePathThroughPoses(const QVariantList& points);

  /**
   * @brief Navigate to a goal pose
   *
   * @param x Goal x in map frame
   * @param y Goal y in map frame
   * @param theta Goal yaw in radians
   *
   * @return true if the request was sent, false otherwise
   */
  Q_INVOKABLE bool navigateToPose(double x, double y, double theta);

  /**
   * @brief Navigate through multiple poses
   *
   * @param points Goal poses in map frame as list of maps with keys x, y, and optional theta
   *
   * @return true if the request was sent, false otherwise
   */
  Q_INVOKABLE bool navigateThroughPoses(const QVariantList& points);

  /**
   * @brief Cancel the active navigate to pose action
   *
   * @return true if the cancel request was sent successfully, false otherwise
   */
  Q_INVOKABLE bool cancelNavigateToPose();

signals:
  void statusChanged();                                        // Emitted when the status changes
  void requestFinished(bool success);                          // Emitted when a command request finishes
  void pathComputed(bool success, const QVariantList& points); // Emitted when path action finishes
  void navigationFinished(int resultCode);                     // Emitted when navigate action finishes

private slots:
  void onPoll(); // Polling slot to update the status

private:
  /**
   * @brief Synchronize the status from the RobotServiceClient
   */
  void syncFromClient();

  /**
   * @brief Convert the CommandStatus from the RobotServiceClient to the Bridge Status
   *
   * @param status The CommandStatus from the RobotServiceClient
   *
   * @return The corresponding Bridge Status
   */
  Status toBridgeStatus(ROBOGait::ros::service::RobotServiceClient::CommandStatus status);

  /**
   * @brief Convert RobotServiceClient navigation result to bridge navigation result
   *
   * @param result Navigation result from RobotServiceClient
   *
   * @return Equivalent bridge navigation result
   */
  static NavigationResult toBridgeNavigationResult(ROBOGait::ros::service::RobotServiceClient::NavigationResult result);

  int status_;                 /**< The current status of the command executor */
  QString active_command_key_; /**< The active command key */
  QTimer poll_timer_;          /**< The timer used for polling the command executor status */

  static constexpr int POLL_INTERVAL_MS = 200; /**< The polling interval in milliseconds */
};
} // namespace service
} // namespace qml
} // namespace ROBOGait
