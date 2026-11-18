#include "Themes/ThemeMap.hpp"

using namespace ROBOGait::settings;

ThemeMap::ThemeMap(QObject* parent) : QObject(parent) {}

QColor ThemeMap::getAppBackground() const { return QColor(APP_BACKGROUND); }

QColor ThemeMap::getWhite() const { return QColor(WHITE); }

QColor ThemeMap::getPanel() const { return QColor(PANEL); }

QColor ThemeMap::getPanelBorder() const { return QColor(PANEL_BORDER); }

QColor ThemeMap::getPanelHeader() const { return QColor(PANEL_HEADER); }

QColor ThemeMap::getActionButton() const { return QColor(ACTION_BUTTON); }

QColor ThemeMap::getEmergencyStopLatchedBg() const { return QColor(EMERGENCY_STOP_LATCHED_BG); }

QColor ThemeMap::getEmergencyStopLatchedBorder() const { return QColor(EMERGENCY_STOP_LATCHED_BORDER); }

QColor ThemeMap::getWarningTitle() const { return QColor(WARNING_TITLE); }

QColor ThemeMap::getPathHeadBlue() const { return QColor(PATH_HEAD_BLUE); }

QColor ThemeMap::getPathColorPrimary() const { return QColor(PATH_COLOR_PRIMARY); }

QColor ThemeMap::getPathColorSecondary() const { return QColor(PATH_COLOR_SECONDARY); }

QColor ThemeMap::getPathColorTertiary() const { return QColor(PATH_COLOR_TERTIARY); }

QColor ThemeMap::getRobotBody() const { return QColor(ROBOT_BODY); }

QColor ThemeMap::getRobotWheel() const { return QColor(ROBOT_WHEEL); }

QColor ThemeMap::getRobotHead() const { return QColor(ROBOT_HEAD); }

QColor ThemeMap::getLaser() const { return QColor(LASER); }

QColor ThemeMap::getParticle() const { return QColor(PARTICLE); }

QColor ThemeMap::getMapUnknown() const { return QColor(MAP_UNKNOWN); }

QColor ThemeMap::getMapFree() const { return QColor(MAP_FREE); }

QColor ThemeMap::getMapOccupied() const { return QColor(MAP_OCCUPIED); }
