#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include "Loader/YamlLoader.hpp"
#include "Settings/UiSizingSettings.hpp"

using namespace ROBOGait::settings;

UiSizingSettings& UiSizingSettings::getInstance()
{
  static UiSizingSettings instance;
  return instance;
}

double UiSizingSettings::getAuthModeHeight() const { return auth_mode_height_; }

double UiSizingSettings::getInputHeight() const { return input_height_; }

double UiSizingSettings::getComboHeight() const { return combo_height_; }

double UiSizingSettings::getActionButtonHeight() const { return action_button_height_; }

double UiSizingSettings::getCalibrationFactor() const { return calibration_factor_; }

double UiSizingSettings::getMinInteractivePx() const { return min_interactive_px_; }

double UiSizingSettings::getMaxInteractivePx() const { return max_interactive_px_; }

double UiSizingSettings::px(double value, double pixel_density_px_per_mm) const
{
  if (value <= 0.0)
  {
    return 0.0;
  }

  const double density = (pixel_density_px_per_mm > 0.0) ? pixel_density_px_per_mm : fallbackPixelDensityPxPerMm();
  return value * density * calibration_factor_;
}

double UiSizingSettings::interactivePx(double value, double pixel_density_px_per_mm) const
{
  const double out = px(value, pixel_density_px_per_mm);

  if (out < min_interactive_px_)
  {
    return min_interactive_px_;
  }
  if (out > max_interactive_px_)
  {
    return max_interactive_px_;
  }
  return out;
}

void UiSizingSettings::initializeDefaults()
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qWarning() << "[UiSizingSettings::initializeDefaults] YAML configuration not loaded, using defaults";
    return;
  }

  auth_mode_height_ = sanitizePositive(yaml_loader.getValue<double>("ui.register.auth_mode_height", DEFAULT_AUTH_MODE_HEIGHT), DEFAULT_AUTH_MODE_HEIGHT);

  input_height_ = sanitizePositive(yaml_loader.getValue<double>("ui.register.input_height", DEFAULT_INPUT_HEIGHT), DEFAULT_INPUT_HEIGHT);

  combo_height_ = sanitizePositive(yaml_loader.getValue<double>("ui.register.combo_height", DEFAULT_COMBO_HEIGHT), DEFAULT_COMBO_HEIGHT);

  action_button_height_ =
      sanitizePositive(yaml_loader.getValue<double>("ui.register.action_button_height", DEFAULT_ACTION_BUTTON_HEIGHT), DEFAULT_ACTION_BUTTON_HEIGHT);

  calibration_factor_ =
      sanitizePositive(yaml_loader.getValue<double>("ui.register.calibration_factor", DEFAULT_CALIBRATION_FACTOR), DEFAULT_CALIBRATION_FACTOR);

  min_interactive_px_ =
      sanitizePositive(yaml_loader.getValue<double>("ui.register.min_interactive_px", DEFAULT_MIN_INTERACTIVE_PX), DEFAULT_MIN_INTERACTIVE_PX);

  max_interactive_px_ =
      sanitizePositive(yaml_loader.getValue<double>("ui.register.max_interactive_px", DEFAULT_MAX_INTERACTIVE_PX), DEFAULT_MAX_INTERACTIVE_PX);

  if (max_interactive_px_ < min_interactive_px_)
  {
    qWarning() << "[UiSizingSettings::initializeDefaults] max_interactive_px < min_interactive_px, swapping";
    const double tmp = max_interactive_px_;
    max_interactive_px_ = min_interactive_px_;
    min_interactive_px_ = tmp;
  }

  emit sizingChanged();
}

UiSizingSettings::UiSizingSettings() :
    auth_mode_height_(DEFAULT_AUTH_MODE_HEIGHT),
    input_height_(DEFAULT_INPUT_HEIGHT),
    combo_height_(DEFAULT_COMBO_HEIGHT),
    action_button_height_(DEFAULT_ACTION_BUTTON_HEIGHT),
    calibration_factor_(DEFAULT_CALIBRATION_FACTOR),
    min_interactive_px_(DEFAULT_MIN_INTERACTIVE_PX),
    max_interactive_px_(DEFAULT_MAX_INTERACTIVE_PX)
{
  qInfo() << "[UiSizingSettings::UiSizingSettings] UiSizingSettings created";
}

double UiSizingSettings::sanitizePositive(double value, double fallback) const { return (value > 0.0) ? value : fallback; }

double UiSizingSettings::fallbackPixelDensityPxPerMm() const
{
  QScreen* screen = QGuiApplication::primaryScreen();
  if (!screen)
  {
    return DEFAULT_FALLBACK_DENSITY_PX_PER_MM;
  }

  const double dpi = screen->physicalDotsPerInch();
  if (dpi <= 0.0)
  {
    return DEFAULT_FALLBACK_DENSITY_PX_PER_MM;
  }

  return dpi / MILLIMETERS_PER_INCH;
}