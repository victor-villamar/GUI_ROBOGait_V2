#pragma once

#include <memory>

#include <QApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QString>
#include <QTranslator>

#include "DataBase/DataBaseManager.hpp"
#include "Ros/RosNodeManager.hpp"
#include "Settings/DeveloperSettings.hpp"
#include "Settings/TimeoutSettings.hpp"
#include "Settings/UiSizingSettings.hpp"
#include "User/UserSession.hpp"

namespace ROBOGait
{
namespace core
{

/**
 * @brief Main application class for RoboGait GUI
 *
 * This class manages the application lifecycle, initializes all subsystems,
 * and provides access to core managers and services.
 *
 * Note: Uses QApplication (not QGuiApplication) to support Qt Widgets
 * (QGraphicsView/QGraphicsScene) for map visualization.
 */
class RoboGaitApplication : public QApplication
{
  Q_OBJECT

public:
  /**
   * @brief Constructor of RoboGaitApplication class
   * @param argc Argument count
   * @param argv Argument values
   */
  RoboGaitApplication(int& argc, char* argv[]);

  /**
   * @brief Destructor of RoboGaitApplication class
   */
  ~RoboGaitApplication();

  /**
   * @brief Initialize common functionalities (QML types, metatypes, etc.)
   *
   * This method registers all QML types, metatypes and other common
   * initialization that must happen before creating managers.
   */
  void initCommon();

  /**
   * @brief Initialize all managers and subsystems
   *
   * @return true if initialization succeeded, false otherwise
   */
  bool initialize();

  /**
   * @brief Initialize for normal application boot
   *
   * Sets up the QML engine, context properties, and loads the main QML file.
   *
   * @return true if initialization succeeded, false otherwise
   */
  bool initForNormalAppBoot();

  /**
   * @brief Get the ROS node manager instance
   *
   * @return Pointer to RosNodeManager
   */
  ROBOGait::ros::manager::RosNodeManager* rosNodeManager();

  /**
   * @brief Get the database manager singleton instance
   *
   * @return Pointer to DataBaseManager
   */
  ROBOGait::db::DataBaseManager* databaseManager();

  /**
   * @brief Get the user session instance
   *
   * @return Pointer to UserSession
   */
  ROBOGait::session::UserSession* userSession();

  /**
   * @brief Get the developer settings singleton instance
   *
   * @return Pointer to DeveloperSettings
   */
  ROBOGait::settings::DeveloperSettings* developerSettings();

  /**
   * @brief Get the UI sizing settings singleton instance
   *
   * @return Pointer to UiSizingSettings
   */
  ROBOGait::settings::UiSizingSettings* uiSizingSettings();

  /**
   * @brief Get the timeout settings singleton instance
   *
   * @return Pointer to TimeoutSettings
   */
  ROBOGait::settings::TimeoutSettings* timeoutSettings();

  /**
   * @brief Get the QML application engine

   * @return Pointer to QQmlApplicationEngine
   */
  QQmlApplicationEngine* qmlEngine();

  /**
   * @brief Get the singleton instance of RoboGaitApplication
   *
   * @return Pointer to the application instance
   */
  static RoboGaitApplication* instance();

private slots:
  /**
   * @brief Handle developer settings changes
   *
   * This slot is called when DeveloperSettings::settingsApplied() is emitted.
   * It checks if ROS domain ID changed and restarts the ROS node if needed.
   */
  void onDeveloperSettingsApplied();

private:
  /**
   * @brief Setup application translator for internationalization
   */
  void setupTranslator();

  /**
   * @brief Setup database (copy template if needed and initialize)
   *
   * @param db_path Path to the database file from configuration
   *
   * @return true if setup succeeded, false otherwise
   */
  bool setupDatabase(const QString& db_path);

  /**
   * @brief Setup QML context properties
   */
  void setupQmlContext();

  /**
   * @brief Connect application signals and slots
   */
  void connectSignals();

  std::unique_ptr<ROBOGait::ros::manager::RosNodeManager> ros_node_manager_; /**< ROS node manager instance */
  std::unique_ptr<ROBOGait::session::UserSession> user_session_;             /**< User session instance */

  QQmlApplicationEngine* qml_app_engine_; /**< QML application engine */
  QTranslator translator_;                /**< Application translator for i18n */

  int argc_;    /**< Argument count (stored for ROS restart) */
  char** argv_; /**< Argument values (stored for ROS restart) */

  static RoboGaitApplication* app_instance_; /**< Singleton instance */
};

} // namespace core
} // namespace ROBOGait

/**
 * @brief Global accessor for RoboGaitApplication instance
 * @return Pointer to the application instance
 */
ROBOGait::core::RoboGaitApplication* roboGaitApp();
