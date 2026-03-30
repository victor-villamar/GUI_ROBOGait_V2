#pragma once

#include <QObject>

namespace ROBOGait
{
namespace settings
{
class UiSizingSettings : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Get the singleton instance
   *
   * @return Reference to the singleton instance
   */
  static UiSizingSettings& getInstance();

  /**
   * @brief Delete copy constructor and assignment operator
   */
  UiSizingSettings(const UiSizingSettings&) = delete;
  UiSizingSettings& operator=(const UiSizingSettings&) = delete;

  // clang-format off
  Q_PROPERTY(double authModeHeight
             READ getAuthModeHeight
             NOTIFY sizingChanged)
  
  Q_PROPERTY(double inputHeight
             READ getInputHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double comboHeight
             READ getComboHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double actionButtonHeight
             READ getActionButtonHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double calibrationFactor
             READ getCalibrationFactor
             NOTIFY sizingChanged)

  Q_PROPERTY(double minInteractivePx
             READ getMinInteractivePx
             NOTIFY sizingChanged)

  Q_PROPERTY(double maxInteractivePx
             READ getMaxInteractivePx
             NOTIFY sizingChanged)
  // clang-format on

  double getAuthModeHeight() const;
  double getInputHeight() const;
  double getComboHeight() const;
  double getActionButtonHeight() const;
  double getCalibrationFactor() const;
  double getMinInteractivePx() const;
  double getMaxInteractivePx() const;

  Q_INVOKABLE double px(double value, double pixel_density_px_per_mm) const;

  Q_INVOKABLE double interactivePx(double value, double pixel_density_px_per_mm) const;

  void initializeDefaults();

signals:
  void sizingChanged();

private:
  UiSizingSettings();
  ~UiSizingSettings() override = default;

  double sanitizePositive(double value, double fallback) const;
  double fallbackPixelDensityPxPerMm() const;

  double auth_mode_height_;
  double input_height_;
  double combo_height_;
  double action_button_height_;
  double calibration_factor_;
  double min_interactive_px_;
  double max_interactive_px_;

  static constexpr double DEFAULT_AUTH_MODE_HEIGHT = 12.0;
  static constexpr double DEFAULT_INPUT_HEIGHT = 12.0;
  static constexpr double DEFAULT_COMBO_HEIGHT = 12.0;
  static constexpr double DEFAULT_ACTION_BUTTON_HEIGHT = 12.0;

  static constexpr double DEFAULT_CALIBRATION_FACTOR = 1.0;
  static constexpr double DEFAULT_MIN_INTERACTIVE_PX = 44.0;
  static constexpr double DEFAULT_MAX_INTERACTIVE_PX = 96.0;

  static constexpr double DEFAULT_FALLBACK_DENSITY_PX_PER_MM = 3.779527559;
  static constexpr double MILLIMETERS_PER_INCH = 25.4;
};
} // namespace settings
} // namespace ROBOGait