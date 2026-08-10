#include "Themes/ThemeOrientationWheel.hpp"

using namespace ROBOGait::settings;

ThemeOrientationWheel::ThemeOrientationWheel(QObject* parent) : QObject(parent) {}

QColor ThemeOrientationWheel::getWhite() const { return QColor(WHITE); }

QColor ThemeOrientationWheel::getPanelEnabled() const { return QColor(PANEL_ENABLED); }

QColor ThemeOrientationWheel::getPanelDisabled() const { return QColor(PANEL_DISABLED); }

QColor ThemeOrientationWheel::getBorderEnabled() const { return QColor(BORDER_ENABLED); }

QColor ThemeOrientationWheel::getBorderDisabled() const { return QColor(BORDER_DISABLED); }

QColor ThemeOrientationWheel::getHeaderEnabled() const { return QColor(HEADER_ENABLED); }

QColor ThemeOrientationWheel::getHeaderDisabled() const { return QColor(HEADER_DISABLED); }

QColor ThemeOrientationWheel::getMarkerDisabled() const { return QColor(MARKER_DISABLED); }
