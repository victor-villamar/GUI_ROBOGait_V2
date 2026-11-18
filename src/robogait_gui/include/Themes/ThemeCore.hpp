#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeCore : public QObject
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

  Q_PROPERTY(QColor dark
             READ getDark
             CONSTANT)

  Q_PROPERTY(QColor overlayDim
             READ getOverlayDim
             CONSTANT)

  Q_PROPERTY(QColor scrollTrack
             READ getScrollTrack
             CONSTANT)
  // clang-format on

  explicit ThemeCore(QObject* parent = nullptr);

  QColor getAppBackground() const;
  QColor getWhite() const;
  QColor getDark() const;
  QColor getOverlayDim() const;
  QColor getScrollTrack() const;

private:
  static constexpr const char* APP_BACKGROUND = "#518bb7";
  static constexpr const char* WHITE = "#ffffff";
  static constexpr const char* DARK = "#000000";
  static constexpr const char* OVERLAY_DIM = "#A0505050";
  static constexpr const char* SCROLL_TRACK = "#0f3f63";
};
} // namespace settings
} // namespace ROBOGait
