#include "Themes/ThemeRobotConnection.hpp"

using namespace ROBOGait::settings;

ThemeRobotConnection::ThemeRobotConnection(QObject* parent) : QObject(parent) {}

QColor ThemeRobotConnection::getBackground() const { return QColor(BACKGROUND); }

QColor ThemeRobotConnection::getWhite() const { return QColor(WHITE); }

QColor ThemeRobotConnection::getPrimary() const { return QColor(PRIMARY); }

QColor ThemeRobotConnection::getAccent() const { return QColor(ACCENT); }

QColor ThemeRobotConnection::getRobotCardSelected() const { return QColor(ROBOT_CARD_SELECTED); }

QColor ThemeRobotConnection::getRobotCardNormal() const { return QColor(ROBOT_CARD_NORMAL); }

QColor ThemeRobotConnection::getRobotLabel() const { return QColor(ROBOT_LABEL); }
