#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeSettings : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor overlayDim
             READ getOverlayDim
             CONSTANT)

  Q_PROPERTY(QColor white
             READ getWhite
             CONSTANT)

  Q_PROPERTY(QColor black
             READ getBlack
             CONSTANT)

  Q_PROPERTY(QColor primary
             READ getPrimary
             CONSTANT)

  Q_PROPERTY(QColor primaryDark
             READ getPrimaryDark
             CONSTANT)

  Q_PROPERTY(QColor lightBlue
             READ getLightBlue
             CONSTANT)

  Q_PROPERTY(QColor closeButtonPressedBg
             READ getCloseButtonPressedBg
             CONSTANT)

  Q_PROPERTY(QColor tabUnselectedBg
             READ getTabUnselectedBg
             CONSTANT)

  Q_PROPERTY(QColor applyProgressFill
             READ getApplyProgressFill
             CONSTANT)

  Q_PROPERTY(QColor sectionTitleDisabled
             READ getSectionTitleDisabled
             CONSTANT)

  Q_PROPERTY(QColor groupDescription
             READ getGroupDescription
             CONSTANT)

  Q_PROPERTY(QColor statusOk
             READ getStatusOk
             CONSTANT)

  Q_PROPERTY(QColor statusWarning
             READ getStatusWarning
             CONSTANT)

  // clang-format on

  explicit ThemeSettings(QObject* parent = nullptr);

  QColor getOverlayDim() const;
  QColor getWhite() const;
  QColor getBlack() const;
  QColor getPrimary() const;
  QColor getPrimaryDark() const;
  QColor getLightBlue() const;
  QColor getCloseButtonPressedBg() const;
  QColor getTabUnselectedBg() const;
  QColor getApplyProgressFill() const;
  QColor getSectionTitleDisabled() const;
  QColor getGroupDescription() const;
  QColor getStatusOk() const;
  QColor getStatusWarning() const;

private:
  static constexpr const char* OVERLAY_DIM = "#A0505050";
  static constexpr const char* WHITE = "#ffffff";
  static constexpr const char* BLACK = "#000000";
  static constexpr const char* PRIMARY = "#518bb7";
  static constexpr const char* PRIMARY_DARK = "#045671";
  static constexpr const char* LIGHT_BLUE = "#a9cfe8";
  static constexpr const char* CLOSE_BUTTON_PRESSED_BG = "#518bb7";
  static constexpr const char* TAB_UNSELECTED_BG = "#e6f2f8";
  static constexpr const char* APPLY_PROGRESS_FILL = "#00C8FF";
  static constexpr const char* SECTION_TITLE_DISABLED = "#999999";
  static constexpr const char* GROUP_DESCRIPTION = "#666666";
  static constexpr const char* STATUS_OK = "#2E7D32";
  static constexpr const char* STATUS_WARNING = "#E65100";
};
} // namespace settings
} // namespace ROBOGait
