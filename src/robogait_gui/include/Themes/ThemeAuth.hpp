#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeAuth : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor placeholderText
             READ getPlaceholderText
             CONSTANT)
            
  Q_PROPERTY(QColor buttonBackground
             READ getButtonBackground
             CONSTANT)

  Q_PROPERTY(QColor comboBackground
             READ getComboBackground
             CONSTANT)

  Q_PROPERTY(QColor comboBackgroundHighlighted
             READ getComboBackgroundHighlighted
             CONSTANT)

  Q_PROPERTY(QColor light
             READ getLight
             CONSTANT)

  Q_PROPERTY(QColor dropdownPrimary
             READ getDropdownPrimary
             CONSTANT)

  Q_PROPERTY(QColor dropdownPressedBackground
             READ getDropdownPressedBackground
             CONSTANT)

  // clang-format on

  explicit ThemeAuth(QObject* parent = nullptr);

  QColor getPlaceholderText() const;
  QColor getButtonBackground() const;
  QColor getComboBackground() const;
  QColor getComboBackgroundHighlighted() const;
  QColor getLight() const;
  QColor getDropdownPrimary() const;
  QColor getDropdownPressedBackground() const;

private:
  static constexpr const char* PLACEHOLDER_TEXT = "#808080";
  static constexpr const char* BUTTON_BACKGROUND = "#aed2ea";
  static constexpr const char* COMBO_BACKGROUND = "#333333";
  static constexpr const char* COMBO_BACKGROUND_HIGHLIGHTED = "#4a4a4a";
  static constexpr const char* LIGHT = "#ffffff";
  static constexpr const char* DROPDOWN_PRIMARY = "#045671";
  static constexpr const char* DROPDOWN_PRESSED_BACKGROUND = "#00C8FF";
};
} // namespace settings
} // namespace ROBOGait
