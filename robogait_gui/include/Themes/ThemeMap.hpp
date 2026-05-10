#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeMap : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor appBackground
             READ getAppBackground
             CONSTANT)

  Q_PROPERTY(QColor white
             READ getWhite
             CONSTANT)

  Q_PROPERTY(QColor panel
             READ getPanel
             CONSTANT)

  Q_PROPERTY(QColor panelBorder
             READ getPanelBorder
             CONSTANT)

  Q_PROPERTY(QColor panelHeader
             READ getPanelHeader
             CONSTANT)

  Q_PROPERTY(QColor actionButton
             READ getActionButton
             CONSTANT)

  Q_PROPERTY(QColor emergencyStopLatchedBg
             READ getEmergencyStopLatchedBg
             CONSTANT)

  Q_PROPERTY(QColor emergencyStopLatchedBorder
             READ getEmergencyStopLatchedBorder
             CONSTANT)

  Q_PROPERTY(QColor warningTitle
             READ getWarningTitle
             CONSTANT)

  Q_PROPERTY(QColor pathHeadBlue
             READ getPathHeadBlue
             CONSTANT)

  Q_PROPERTY(QColor pathColorPrimary
             READ getPathColorPrimary
             CONSTANT)

  Q_PROPERTY(QColor pathColorSecondary
             READ getPathColorSecondary
             CONSTANT)

  Q_PROPERTY(QColor pathColorTertiary
             READ getPathColorTertiary
             CONSTANT)

  Q_PROPERTY(QColor robotBody
             READ getRobotBody
             CONSTANT)

  Q_PROPERTY(QColor robotWheel
             READ getRobotWheel
             CONSTANT)

  Q_PROPERTY(QColor robotHead
             READ getRobotHead
             CONSTANT)

  Q_PROPERTY(QColor laser
             READ getLaser
             CONSTANT)

  Q_PROPERTY(QColor particle
             READ getParticle
             CONSTANT)

  Q_PROPERTY(QColor mapUnknown
             READ getMapUnknown
             CONSTANT)

  Q_PROPERTY(QColor mapFree
             READ getMapFree
             CONSTANT)

  Q_PROPERTY(QColor mapOccupied
             READ getMapOccupied
             CONSTANT)
  // clang-format on

  explicit ThemeMap(QObject* parent = nullptr);

  QColor getAppBackground() const;
  QColor getWhite() const;
  QColor getPanel() const;
  QColor getPanelBorder() const;
  QColor getPanelHeader() const;
  QColor getActionButton() const;
  QColor getEmergencyStopLatchedBg() const;
  QColor getEmergencyStopLatchedBorder() const;
  QColor getWarningTitle() const;
  QColor getPathHeadBlue() const;
  QColor getPathColorPrimary() const;
  QColor getPathColorSecondary() const;
  QColor getPathColorTertiary() const;
  QColor getRobotBody() const;
  QColor getRobotWheel() const;
  QColor getRobotHead() const;
  QColor getLaser() const;
  QColor getParticle() const;
  QColor getMapUnknown() const;
  QColor getMapFree() const;
  QColor getMapOccupied() const;

private:
  static constexpr const char* APP_BACKGROUND = "#518bb7";
  static constexpr const char* WHITE = "#ffffff";
  static constexpr const char* PANEL = "#2c5f7c";
  static constexpr const char* PANEL_BORDER = "#6aa3c8";
  static constexpr const char* PANEL_HEADER = "#1a3a4a";
  static constexpr const char* ACTION_BUTTON = "#3a7fa0";
  static constexpr const char* EMERGENCY_STOP_LATCHED_BG = "#7a8a93";
  static constexpr const char* EMERGENCY_STOP_LATCHED_BORDER = "#cbd6dc";
  static constexpr const char* WARNING_TITLE = "#c52020";
  static constexpr const char* PATH_HEAD_BLUE = "#3b82f6";
  static constexpr const char* PATH_COLOR_PRIMARY = "#9118DB";
  static constexpr const char* PATH_COLOR_SECONDARY = "#F57927";
  static constexpr const char* PATH_COLOR_TERTIARY = "#18DB22";
  static constexpr const char* ROBOT_BODY = "#4b5056";
  static constexpr const char* ROBOT_WHEEL = "#15181f";
  static constexpr const char* ROBOT_HEAD = "#dc1414";
  static constexpr const char* LASER = "#ff0000";
  static constexpr const char* PARTICLE = "#ff8000";
  static constexpr const char* MAP_UNKNOWN = "#1a3a4a";
  static constexpr const char* MAP_FREE = "#a9cfe8";
  static constexpr const char* MAP_OCCUPIED = "#ffffff";
};
} // namespace settings
} // namespace ROBOGait
