#pragma once

#include <QColor>
#include <QObject>

namespace ROBOGait
{
namespace settings
{
class ThemeSelection : public QObject
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

  Q_PROPERTY(QColor black
             READ getBlack
             CONSTANT)

  Q_PROPERTY(QColor primary
             READ getPrimary
             CONSTANT)

  Q_PROPERTY(QColor primaryDark
             READ getPrimaryDark
             CONSTANT)

  Q_PROPERTY(QColor lightBlue
             READ getLightBlue
             CONSTANT)

  Q_PROPERTY(QColor overlayDim
             READ getOverlayDim
             CONSTANT)

  Q_PROPERTY(QColor closeButtonPressedBackground
             READ getCloseButtonPressedBackground
             CONSTANT)

  Q_PROPERTY(QColor detailsCardBackground
             READ getDetailsCardBackground
             CONSTANT)

  Q_PROPERTY(QColor detailsCardBorder
             READ getDetailsCardBorder
             CONSTANT)

  Q_PROPERTY(QColor previewBackground
             READ getPreviewBackground
             CONSTANT)

  Q_PROPERTY(QColor previewBorder
             READ getPreviewBorder
             CONSTANT)

  // clang-format on

  explicit ThemeSelection(QObject* parent = nullptr);

  QColor getBackground() const;
  QColor getWhite() const;
  QColor getBlack() const;
  QColor getPrimary() const;
  QColor getPrimaryDark() const;
  QColor getLightBlue() const;
  QColor getOverlayDim() const;
  QColor getCloseButtonPressedBackground() const;
  QColor getDetailsCardBackground() const;
  QColor getDetailsCardBorder() const;
  QColor getPreviewBackground() const;
  QColor getPreviewBorder() const;

private:
  static constexpr const char* BACKGROUND = "#518bb7";
  static constexpr const char* WHITE = "#ffffff";
  static constexpr const char* BLACK = "#000000";
  static constexpr const char* PRIMARY = "#518bb7";
  static constexpr const char* PRIMARY_DARK = "#045671";
  static constexpr const char* LIGHT_BLUE = "#aed2ea";
  static constexpr const char* OVERLAY_DIM = "#A0505050";
  static constexpr const char* CLOSE_BUTTON_PRESSED_BACKGROUND = "#a9cfe8";
  static constexpr const char* DETAILS_CARD_BACKGROUND = "#e9e9e9";
  static constexpr const char* DETAILS_CARD_BORDER = "#235c87";
  static constexpr const char* PREVIEW_BACKGROUND = "#d9d9d9";
  static constexpr const char* PREVIEW_BORDER = "#c0c0c0";
};
} // namespace settings
} // namespace ROBOGait
