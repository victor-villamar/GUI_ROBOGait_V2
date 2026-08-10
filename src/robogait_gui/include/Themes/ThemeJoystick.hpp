#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeJoystick : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor baseFill
             READ getBaseFill
             CONSTANT)

  Q_PROPERTY(QColor light
             READ getLight
             CONSTANT)

  Q_PROPERTY(QColor stickFill
             READ getStickFill
             CONSTANT)
  // clang-format on

  explicit ThemeJoystick(QObject* parent = nullptr);

  QColor getBaseFill() const;
  QColor getLight() const;
  QColor getStickFill() const;

private:
  static constexpr const char* BASE_FILL = "#a9cfe8";
  static constexpr const char* LIGHT = "#ffffff";
  static constexpr const char* STICK_FILL = "#045671";
};
} // namespace settings
} // namespace ROBOGait
