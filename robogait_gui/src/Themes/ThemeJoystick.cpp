#include "Themes/ThemeJoystick.hpp"

using namespace ROBOGait::settings;

ThemeJoystick::ThemeJoystick(QObject* parent) : QObject(parent) {}

QColor ThemeJoystick::getBaseFill() const { return QColor(BASE_FILL); }

QColor ThemeJoystick::getLight() const { return QColor(LIGHT); }

QColor ThemeJoystick::getStickFill() const { return QColor(STICK_FILL); }
