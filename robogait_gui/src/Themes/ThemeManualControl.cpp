#include "Themes/ThemeManualControl.hpp"

using namespace ROBOGait::settings;

ThemeManualControl::ThemeManualControl(QObject* parent) : QObject(parent) {}

QColor ThemeManualControl::getBackground() const { return QColor(BACKGROUND); }

QColor ThemeManualControl::getSectionTitle() const { return QColor(SECTION_TITLE); }

QColor ThemeManualControl::getSectionTitleShadow() const { return QColor(SECTION_TITLE_SHADOW); }

QColor ThemeManualControl::getLight() const { return QColor(LIGHT); }

QColor ThemeManualControl::getWarningTitle() const { return QColor(WARNING_TITLE); }
