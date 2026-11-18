#include "Themes/ThemeAuth.hpp"

using namespace ROBOGait::settings;

ThemeAuth::ThemeAuth(QObject* parent) : QObject(parent) {}

QColor ThemeAuth::getPlaceholderText() const { return QColor(PLACEHOLDER_TEXT); }

QColor ThemeAuth::getButtonBackground() const { return QColor(BUTTON_BACKGROUND); }

QColor ThemeAuth::getComboBackground() const { return QColor(COMBO_BACKGROUND); }

QColor ThemeAuth::getComboBackgroundHighlighted() const { return QColor(COMBO_BACKGROUND_HIGHLIGHTED); }

QColor ThemeAuth::getLight() const { return QColor(LIGHT); }

QColor ThemeAuth::getDropdownPrimary() const { return QColor(DROPDOWN_PRIMARY); }

QColor ThemeAuth::getDropdownPressedBackground() const { return QColor(DROPDOWN_PRESSED_BACKGROUND); }
