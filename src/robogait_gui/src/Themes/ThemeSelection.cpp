#include "Themes/ThemeSelection.hpp"

using namespace ROBOGait::settings;

ThemeSelection::ThemeSelection(QObject* parent) : QObject(parent) {}

QColor ThemeSelection::getBackground() const { return QColor(BACKGROUND); }

QColor ThemeSelection::getWhite() const { return QColor(WHITE); }

QColor ThemeSelection::getBlack() const { return QColor(BLACK); }

QColor ThemeSelection::getPrimary() const { return QColor(PRIMARY); }

QColor ThemeSelection::getPrimaryDark() const { return QColor(PRIMARY_DARK); }

QColor ThemeSelection::getLightBlue() const { return QColor(LIGHT_BLUE); }

QColor ThemeSelection::getOverlayDim() const { return QColor(OVERLAY_DIM); }

QColor ThemeSelection::getCloseButtonPressedBackground() const { return QColor(CLOSE_BUTTON_PRESSED_BACKGROUND); }

QColor ThemeSelection::getDetailsCardBackground() const { return QColor(DETAILS_CARD_BACKGROUND); }

QColor ThemeSelection::getDetailsCardBorder() const { return QColor(DETAILS_CARD_BORDER); }

QColor ThemeSelection::getPreviewBackground() const { return QColor(PREVIEW_BACKGROUND); }

QColor ThemeSelection::getPreviewBorder() const { return QColor(PREVIEW_BORDER); }
