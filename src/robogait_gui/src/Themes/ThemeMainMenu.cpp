#include "Themes/ThemeMainMenu.hpp"

using namespace ROBOGait::settings;

ThemeMainMenu::ThemeMainMenu(QObject* parent) : QObject(parent) {}

QColor ThemeMainMenu::getBackground() const { return QColor(BACKGROUND); }

QColor ThemeMainMenu::getTextPrimary() const { return QColor(TEXT_PRIMARY); }

QColor ThemeMainMenu::getStatusMapActive() const { return QColor(STATUS_MAP_ACTIVE); }

QColor ThemeMainMenu::getStatusError() const { return QColor(STATUS_ERROR); }
