#include "Themes/ThemeSettings.hpp"

using namespace ROBOGait::settings;

ThemeSettings::ThemeSettings(QObject* parent) : QObject(parent) {}

QColor ThemeSettings::getOverlayDim() const { return QColor(OVERLAY_DIM); }

QColor ThemeSettings::getWhite() const { return QColor(WHITE); }

QColor ThemeSettings::getBlack() const { return QColor(BLACK); }

QColor ThemeSettings::getPrimary() const { return QColor(PRIMARY); }

QColor ThemeSettings::getPrimaryDark() const { return QColor(PRIMARY_DARK); }

QColor ThemeSettings::getLightBlue() const { return QColor(LIGHT_BLUE); }

QColor ThemeSettings::getCloseButtonPressedBg() const { return QColor(CLOSE_BUTTON_PRESSED_BG); }

QColor ThemeSettings::getTabUnselectedBg() const { return QColor(TAB_UNSELECTED_BG); }

QColor ThemeSettings::getApplyProgressFill() const { return QColor(APPLY_PROGRESS_FILL); }

QColor ThemeSettings::getSectionTitleDisabled() const { return QColor(SECTION_TITLE_DISABLED); }

QColor ThemeSettings::getGroupDescription() const { return QColor(GROUP_DESCRIPTION); }

QColor ThemeSettings::getStatusOk() const { return QColor(STATUS_OK); }

QColor ThemeSettings::getStatusWarning() const { return QColor(STATUS_WARNING); }
