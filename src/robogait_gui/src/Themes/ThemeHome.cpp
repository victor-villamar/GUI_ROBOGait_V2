#include "Themes/ThemeHome.hpp"

using namespace ROBOGait::settings;

ThemeHome::ThemeHome(QObject* parent) : QObject(parent) {}

QColor ThemeHome::getStartButtonBg() const { return QColor(START_BUTTON_BG); }

QColor ThemeHome::getLight() const { return QColor(LIGHT); }

QColor ThemeHome::getStartButtonText() const { return QColor(START_BUTTON_TEXT); }
