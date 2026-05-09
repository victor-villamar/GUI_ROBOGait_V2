#include "Themes/ThemeCore.hpp"

using namespace ROBOGait::settings;

ThemeCore::ThemeCore(QObject* parent) : QObject(parent) {}

QColor ThemeCore::getAppBackground() const { return QColor(APP_BACKGROUND); }

QColor ThemeCore::getWhite() const { return QColor(WHITE); }

QColor ThemeCore::getDark() const { return QColor(DARK); }

QColor ThemeCore::getOverlayDim() const { return QColor(OVERLAY_DIM); }

QColor ThemeCore::getScrollTrack() const { return QColor(SCROLL_TRACK); }
