#include "Themes/ThemeDialogsCommon.hpp"

using namespace ROBOGait::settings;

ThemeDialogsCommon::ThemeDialogsCommon(QObject* parent) : QObject(parent) {}

QColor ThemeDialogsCommon::getOverlayDim() const { return QColor(OVERLAY_DIM); }

QColor ThemeDialogsCommon::getLight() const { return QColor(LIGHT); }

QColor ThemeDialogsCommon::getPrimary() const { return QColor(PRIMARY); }

QColor ThemeDialogsCommon::getPanelBackground() const { return QColor(PANEL_BACKGROUND); }

QColor ThemeDialogsCommon::getPanelAccentBorder() const { return QColor(PANEL_ACCENT_BORDER); }

QColor ThemeDialogsCommon::getInfoHover() const { return QColor(INFO_HOVER); }

QColor ThemeDialogsCommon::getInfoText() const { return QColor(INFO_TEXT); }

QColor ThemeDialogsCommon::getTitleText() const { return QColor(TITLE_TEXT); }

QColor ThemeDialogsCommon::getCloseButtonPressed() const { return QColor(CLOSE_BUTTON_PRESSED); }

QColor ThemeDialogsCommon::getAcceptProgress() const { return QColor(ACCEPT_PROGRESS); }

QColor ThemeDialogsCommon::getErrorBackground() const { return QColor(ERROR_BACKGROUND); }

QColor ThemeDialogsCommon::getErrorBorder() const { return QColor(ERROR_BORDER); }

QColor ThemeDialogsCommon::getErrorText() const { return QColor(ERROR_TEXT); }

QColor ThemeDialogsCommon::getDiagnosticDetailsBackground() const { return QColor(DIAGNOSTIC_DETAILS_BACKGROUND); }

QColor ThemeDialogsCommon::getBlack() const { return QColor(BLACK); }
