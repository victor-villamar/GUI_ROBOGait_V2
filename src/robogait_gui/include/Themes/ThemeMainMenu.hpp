#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeMainMenu : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor background
             READ getBackground
             CONSTANT)

  Q_PROPERTY(QColor textPrimary
             READ getTextPrimary
             CONSTANT)

  Q_PROPERTY(QColor statusMapActive
             READ getStatusMapActive
             CONSTANT)

  Q_PROPERTY(QColor statusError
             READ getStatusError
             CONSTANT)
  // clang-format on

  explicit ThemeMainMenu(QObject* parent = nullptr);

  QColor getBackground() const;
  QColor getTextPrimary() const;
  QColor getStatusMapActive() const;
  QColor getStatusError() const;

private:
  static constexpr const char* BACKGROUND = "#518bb7";
  static constexpr const char* TEXT_PRIMARY = "#ffffff";
  static constexpr const char* STATUS_MAP_ACTIVE = "#cce54d";
  static constexpr const char* STATUS_ERROR = "#cc0000";
};
} // namespace settings
} // namespace ROBOGait
