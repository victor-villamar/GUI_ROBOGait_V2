#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeRobotConnection : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor background
             READ getBackground
             CONSTANT)

  Q_PROPERTY(QColor white
             READ getWhite
             CONSTANT)

  Q_PROPERTY(QColor primary
             READ getPrimary
             CONSTANT)

  Q_PROPERTY(QColor accent
             READ getAccent
             CONSTANT)

  Q_PROPERTY(QColor robotCardSelected
             READ getRobotCardSelected
             CONSTANT)

  Q_PROPERTY(QColor robotCardNormal
             READ getRobotCardNormal
             CONSTANT)

  Q_PROPERTY(QColor robotLabel
             READ getRobotLabel
             CONSTANT)

  // clang-format on

  explicit ThemeRobotConnection(QObject* parent = nullptr);

  QColor getBackground() const;
  QColor getWhite() const;
  QColor getPrimary() const;
  QColor getAccent() const;
  QColor getRobotCardSelected() const;
  QColor getRobotCardNormal() const;
  QColor getRobotLabel() const;

private:
  static constexpr const char* BACKGROUND = "#518bb7";
  static constexpr const char* WHITE = "#ffffff";
  static constexpr const char* PRIMARY = "#045671";
  static constexpr const char* ACCENT = "#00C8FF";
  static constexpr const char* ROBOT_CARD_SELECTED = "#ffffff";
  static constexpr const char* ROBOT_CARD_NORMAL = "#a9cfe8";
  static constexpr const char* ROBOT_LABEL = "#4f86b4";
};
} // namespace settings
} // namespace ROBOGait
