#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeHome : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor startButtonBg
             READ getStartButtonBg
             CONSTANT)

  Q_PROPERTY(QColor light
             READ getLight
             CONSTANT)

  Q_PROPERTY(QColor startButtonText
             READ getStartButtonText
             CONSTANT)
  // clang-format on

  explicit ThemeHome(QObject* parent = nullptr);

  QColor getStartButtonBg() const;
  QColor getLight() const;
  QColor getStartButtonText() const;

private:
  static constexpr const char* START_BUTTON_BG = "#aed2ea";
  static constexpr const char* LIGHT = "#ffffff";
  static constexpr const char* START_BUTTON_TEXT = "#518bb7";
};
} // namespace settings
} // namespace ROBOGait
