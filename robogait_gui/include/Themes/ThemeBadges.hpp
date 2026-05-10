#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeBadges : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor badgeBackground
             READ getBadgeBackground
             CONSTANT)

  Q_PROPERTY(QColor light
             READ getLight
             CONSTANT)

  Q_PROPERTY(QColor primary
             READ getPrimary
             CONSTANT)

  Q_PROPERTY(QColor textDisabled
             READ getTextDisabled
             CONSTANT)

  Q_PROPERTY(QColor pressedBackground
             READ getPressedBackground
             CONSTANT)

  // clang-format on

  explicit ThemeBadges(QObject* parent = nullptr);

  QColor getBadgeBackground() const;
  QColor getLight() const;
  QColor getPrimary() const;
  QColor getTextDisabled() const;
  QColor getPressedBackground() const;

private:
  static constexpr const char* BADGE_BACKGROUND = "#a9cfe8";
  static constexpr const char* LIGHT = "#ffffff";
  static constexpr const char* PRIMARY = "#045671";
  static constexpr const char* TEXT_DISABLED = "#8a8a8a";
  static constexpr const char* PRESSED_BACKGROUND = "#00C8FF";
};
} // namespace settings
} // namespace ROBOGait
