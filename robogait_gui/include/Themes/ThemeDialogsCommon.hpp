#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeDialogsCommon : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor overlayDim
             READ getOverlayDim
             CONSTANT)
  Q_PROPERTY(QColor light
             READ getLight
             CONSTANT)
  Q_PROPERTY(QColor primary
             READ getPrimary
             CONSTANT)
  Q_PROPERTY(QColor panelBackground
             READ getPanelBackground
             CONSTANT)
  Q_PROPERTY(QColor panelAccentBorder
             READ getPanelAccentBorder
             CONSTANT)
  Q_PROPERTY(QColor infoHover
             READ getInfoHover
             CONSTANT)
  Q_PROPERTY(QColor infoText
             READ getInfoText
             CONSTANT)
  Q_PROPERTY(QColor titleText
             READ getTitleText
             CONSTANT)
  Q_PROPERTY(QColor closeButtonPressed
             READ getCloseButtonPressed
             CONSTANT)
  Q_PROPERTY(QColor acceptProgress
             READ getAcceptProgress
             CONSTANT)
  Q_PROPERTY(QColor errorBackground
             READ getErrorBackground
             CONSTANT)
  Q_PROPERTY(QColor errorBorder
             READ getErrorBorder
             CONSTANT)
  Q_PROPERTY(QColor errorText
             READ getErrorText
             CONSTANT)
  Q_PROPERTY(QColor diagnosticDetailsBackground
             READ getDiagnosticDetailsBackground
             CONSTANT)
  Q_PROPERTY(QColor black
             READ getBlack
             CONSTANT)
  // clang-format on

  explicit ThemeDialogsCommon(QObject* parent = nullptr);

  QColor getOverlayDim() const;
  QColor getLight() const;
  QColor getPrimary() const;
  QColor getPanelBackground() const;
  QColor getPanelAccentBorder() const;
  QColor getInfoHover() const;
  QColor getInfoText() const;
  QColor getTitleText() const;
  QColor getCloseButtonPressed() const;
  QColor getAcceptProgress() const;
  QColor getErrorBackground() const;
  QColor getErrorBorder() const;
  QColor getErrorText() const;
  QColor getDiagnosticDetailsBackground() const;
  QColor getBlack() const;

private:
  static constexpr const char* OVERLAY_DIM = "#A0505050";
  static constexpr const char* LIGHT = "#ffffff";
  static constexpr const char* PRIMARY = "#045671";
  static constexpr const char* PANEL_BACKGROUND = "#a9cfe8";
  static constexpr const char* PANEL_ACCENT_BORDER = "#235c87";
  static constexpr const char* INFO_HOVER = "#e8f0f6";
  static constexpr const char* INFO_TEXT = "#1e4d70";
  static constexpr const char* TITLE_TEXT = "#4f86b4";
  static constexpr const char* CLOSE_BUTTON_PRESSED = "#518bb7";
  static constexpr const char* ACCEPT_PROGRESS = "#00C8FF";
  static constexpr const char* ERROR_BACKGROUND = "#f8d7da";
  static constexpr const char* ERROR_BORDER = "#f5c6cb";
  static constexpr const char* ERROR_TEXT = "#721c24";
  static constexpr const char* DIAGNOSTIC_DETAILS_BACKGROUND = "#e9e9e9";
  static constexpr const char* BLACK = "#000000";
};
} // namespace settings
} // namespace ROBOGait
