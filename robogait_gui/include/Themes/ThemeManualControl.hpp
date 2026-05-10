#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeManualControl : public QObject
{
  Q_OBJECT

public:
  // clang-format off
  Q_PROPERTY(QColor background
             READ getBackground
             CONSTANT)

  Q_PROPERTY(QColor sectionTitle
             READ getSectionTitle
             CONSTANT)

  Q_PROPERTY(QColor sectionTitleShadow
             READ getSectionTitleShadow
             CONSTANT)

  Q_PROPERTY(QColor light
             READ getLight
             CONSTANT)

  Q_PROPERTY(QColor warningTitle
             READ getWarningTitle
             CONSTANT)
  // clang-format on

  explicit ThemeManualControl(QObject* parent = nullptr);

  QColor getBackground() const;
  QColor getSectionTitle() const;
  QColor getSectionTitleShadow() const;
  QColor getLight() const;
  QColor getWarningTitle() const;

private:
  static constexpr const char* BACKGROUND = "#518bb7";
  static constexpr const char* SECTION_TITLE = "#00C8FF";
  static constexpr const char* SECTION_TITLE_SHADOW = "#e9e9e9";
  static constexpr const char* LIGHT = "#ffffff";
  static constexpr const char* WARNING_TITLE = "#c52020";
};
} // namespace settings
} // namespace ROBOGait
