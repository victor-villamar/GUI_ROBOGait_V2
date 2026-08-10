#include "Themes/ThemeBadges.hpp"

using namespace ROBOGait::settings;

ThemeBadges::ThemeBadges(QObject* parent) : QObject(parent) {}

QColor ThemeBadges::getBadgeBackground() const { return QColor(BADGE_BACKGROUND); }

QColor ThemeBadges::getLight() const { return QColor(LIGHT); }

QColor ThemeBadges::getPrimary() const { return QColor(PRIMARY); }

QColor ThemeBadges::getTextDisabled() const { return QColor(TEXT_DISABLED); }

QColor ThemeBadges::getPressedBackground() const { return QColor(PRESSED_BACKGROUND); }
