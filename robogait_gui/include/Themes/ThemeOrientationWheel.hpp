#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeOrientationWheel : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor white
             READ getWhite
             CONSTANT)

  Q_PROPERTY(QColor panelEnabled
             READ getPanelEnabled
             CONSTANT)

  Q_PROPERTY(QColor panelDisabled
             READ getPanelDisabled
             CONSTANT)

  Q_PROPERTY(QColor borderEnabled
             READ getBorderEnabled
             CONSTANT)

  Q_PROPERTY(QColor borderDisabled
             READ getBorderDisabled
             CONSTANT)

  Q_PROPERTY(QColor headerEnabled
             READ getHeaderEnabled
             CONSTANT)

  Q_PROPERTY(QColor headerDisabled
             READ getHeaderDisabled
             CONSTANT)

  Q_PROPERTY(QColor markerDisabled
             READ getMarkerDisabled
             CONSTANT)
  // clang-format on

  explicit ThemeOrientationWheel(QObject* parent = nullptr);

  QColor getWhite() const;
  QColor getPanelEnabled() const;
  QColor getPanelDisabled() const;
  QColor getBorderEnabled() const;
  QColor getBorderDisabled() const;
  QColor getHeaderEnabled() const;
  QColor getHeaderDisabled() const;
  QColor getMarkerDisabled() const;

private:
  static constexpr const char* WHITE = "#ffffff";
  static constexpr const char* PANEL_ENABLED = "#2c5f7c";
  static constexpr const char* PANEL_DISABLED = "#3b4a55";
  static constexpr const char* BORDER_ENABLED = "#6aa3c8";
  static constexpr const char* BORDER_DISABLED = "#5f707d";
  static constexpr const char* HEADER_ENABLED = "#1a3a4a";
  static constexpr const char* HEADER_DISABLED = "#2e3a43";
  static constexpr const char* MARKER_DISABLED = "#9aa8b1";
};
} // namespace settings
} // namespace ROBOGait
