#pragma once

#include <QObject>

#include "Themes/ThemeAuth.hpp"
#include "Themes/ThemeBadges.hpp"
#include "Themes/ThemeCore.hpp"
#include "Themes/ThemeDialogsCommon.hpp"
#include "Themes/ThemeHome.hpp"
#include "Themes/ThemeJoystick.hpp"
#include "Themes/ThemeMainMenu.hpp"
#include "Themes/ThemeManualControl.hpp"
#include "Themes/ThemeMap.hpp"
#include "Themes/ThemeOrientationWheel.hpp"
#include "Themes/ThemeRobotConnection.hpp"
#include "Themes/ThemeSelection.hpp"
#include "Themes/ThemeSettings.hpp"

namespace ROBOGait
{
namespace settings
{
class AppTheme : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QObject* core
             READ getCore
             CONSTANT)

  Q_PROPERTY(QObject* home
             READ getHome
             CONSTANT)

  Q_PROPERTY(QObject* auth
             READ getAuth
             CONSTANT)

  Q_PROPERTY(QObject* badges
             READ getBadges
             CONSTANT)

  Q_PROPERTY(QObject* settings
             READ getSettings
             CONSTANT)

  Q_PROPERTY(QObject* mainMenu
             READ getMainMenu
             CONSTANT)

  Q_PROPERTY(QObject* map
             READ getMap
             CONSTANT)

  Q_PROPERTY(QObject* selection
             READ getSelection
             CONSTANT)

  Q_PROPERTY(QObject* manualControl
             READ getManualControl
             CONSTANT)

  Q_PROPERTY(QObject* joystick
             READ getJoystick
             CONSTANT)

  Q_PROPERTY(QObject* orientationWheel
             READ getOrientationWheel
             CONSTANT)

  Q_PROPERTY(QObject* robotConnection
             READ getRobotConnection
             CONSTANT)

  Q_PROPERTY(QObject* dialogsCommon
             READ getDialogsCommon
             CONSTANT)
  // clang-format on

  static AppTheme& getInstance();
  AppTheme(const AppTheme&) = delete;
  AppTheme& operator=(const AppTheme&) = delete;

  QObject* getCore();
  QObject* getCore() const;
  QObject* getHome();
  QObject* getHome() const;
  QObject* getAuth();
  QObject* getAuth() const;
  QObject* getBadges();
  QObject* getBadges() const;
  QObject* getSettings();
  QObject* getSettings() const;
  QObject* getMainMenu();
  QObject* getMainMenu() const;
  QObject* getMap();
  QObject* getMap() const;
  QObject* getSelection();
  QObject* getSelection() const;
  QObject* getManualControl();
  QObject* getManualControl() const;
  QObject* getJoystick();
  QObject* getJoystick() const;
  QObject* getOrientationWheel();
  QObject* getOrientationWheel() const;
  QObject* getRobotConnection();
  QObject* getRobotConnection() const;
  QObject* getDialogsCommon();
  QObject* getDialogsCommon() const;

private:
  explicit AppTheme(QObject* parent = nullptr);
  ~AppTheme() override = default;

  ThemeCore core_;
  ThemeHome home_;
  ThemeAuth auth_;
  ThemeBadges badges_;
  ThemeSettings settings_;
  ThemeMainMenu main_menu_;
  ThemeMap map_;
  ThemeSelection selection_;
  ThemeManualControl manual_control_;
  ThemeJoystick joystick_;
  ThemeOrientationWheel orientation_wheel_;
  ThemeRobotConnection robot_connection_;
  ThemeDialogsCommon dialogs_common_;
};
} // namespace settings
} // namespace ROBOGait
