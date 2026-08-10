#include "Themes/AppTheme.hpp"

using namespace ROBOGait::settings;

AppTheme::AppTheme(QObject* parent) :
    QObject(parent),
    core_(this),
    home_(this),
    auth_(this),
    badges_(this),
    settings_(this),
    main_menu_(this),
    map_(this),
    selection_(this),
    manual_control_(this),
    joystick_(this),
    orientation_wheel_(this),
    robot_connection_(this),
    dialogs_common_(this)
{
}

AppTheme& AppTheme::getInstance()
{
  static AppTheme instance;
  return instance;
}

QObject* AppTheme::getCore() { return &core_; }

QObject* AppTheme::getCore() const { return const_cast<ThemeCore*>(&core_); }

QObject* AppTheme::getHome() { return &home_; }

QObject* AppTheme::getHome() const { return const_cast<ThemeHome*>(&home_); }

QObject* AppTheme::getAuth() { return &auth_; }

QObject* AppTheme::getAuth() const { return const_cast<ThemeAuth*>(&auth_); }

QObject* AppTheme::getBadges() { return &badges_; }

QObject* AppTheme::getBadges() const { return const_cast<ThemeBadges*>(&badges_); }

QObject* AppTheme::getSettings() { return &settings_; }

QObject* AppTheme::getSettings() const { return const_cast<ThemeSettings*>(&settings_); }

QObject* AppTheme::getMainMenu() { return &main_menu_; }

QObject* AppTheme::getMainMenu() const { return const_cast<ThemeMainMenu*>(&main_menu_); }

QObject* AppTheme::getMap() { return &map_; }

QObject* AppTheme::getMap() const { return const_cast<ThemeMap*>(&map_); }

QObject* AppTheme::getSelection() { return &selection_; }

QObject* AppTheme::getSelection() const { return const_cast<ThemeSelection*>(&selection_); }

QObject* AppTheme::getManualControl() { return &manual_control_; }

QObject* AppTheme::getManualControl() const { return const_cast<ThemeManualControl*>(&manual_control_); }

QObject* AppTheme::getJoystick() { return &joystick_; }

QObject* AppTheme::getJoystick() const { return const_cast<ThemeJoystick*>(&joystick_); }

QObject* AppTheme::getOrientationWheel() { return &orientation_wheel_; }

QObject* AppTheme::getOrientationWheel() const { return const_cast<ThemeOrientationWheel*>(&orientation_wheel_); }

QObject* AppTheme::getRobotConnection() { return &robot_connection_; }

QObject* AppTheme::getRobotConnection() const { return const_cast<ThemeRobotConnection*>(&robot_connection_); }

QObject* AppTheme::getDialogsCommon() { return &dialogs_common_; }

QObject* AppTheme::getDialogsCommon() const { return const_cast<ThemeDialogsCommon*>(&dialogs_common_); }
