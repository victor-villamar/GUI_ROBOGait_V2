#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include "Loader/YamlLoader.hpp"
#include "Settings/UiSizingSettings.hpp"

using namespace ROBOGait::settings;

struct FieldMapping
{
  const char* yaml_key_;
  double UiSizingSettings::UiSizingValues::* member_;
  double default_value_;
};

static constexpr FieldMapping FIELD_MAPPINGS[] = {
    {"ui.button_height", &UiSizingSettings::UiSizingValues::button_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.input_height", &UiSizingSettings::UiSizingValues::input_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.combo_height", &UiSizingSettings::UiSizingValues::combo_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.dropdown_height", &UiSizingSettings::UiSizingValues::dropdown_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.list_item_height", &UiSizingSettings::UiSizingValues::list_item_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.badge_height", &UiSizingSettings::UiSizingValues::badge_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.badge_menu_item_height", &UiSizingSettings::UiSizingValues::badge_menu_item_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.icon_button_size", &UiSizingSettings::UiSizingValues::icon_button_size_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.icon_glyph_size", &UiSizingSettings::UiSizingValues::icon_glyph_size_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.tab_height", &UiSizingSettings::UiSizingValues::tab_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.checkbox_height", &UiSizingSettings::UiSizingValues::checkbox_height_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.checkbox_indicator_size", &UiSizingSettings::UiSizingValues::checkbox_indicator_size_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
    {"ui.spinbox_button_size", &UiSizingSettings::UiSizingValues::spinbox_button_size_, UiSizingSettings::DEFAULT_COMPONENT_HEIGHT},
};

UiSizingSettings& UiSizingSettings::getInstance()
{
  static UiSizingSettings instance;
  return instance;
}

double UiSizingSettings::getButtonHeight() const { return values_.button_height_; }

double UiSizingSettings::getInputHeight() const { return values_.input_height_; }

double UiSizingSettings::getComboHeight() const { return values_.combo_height_; }

double UiSizingSettings::getDropdownHeight() const { return values_.dropdown_height_; }

double UiSizingSettings::getListItemHeight() const { return values_.list_item_height_; }

double UiSizingSettings::getBadgeHeight() const { return values_.badge_height_; }

double UiSizingSettings::getBadgeMenuItemHeight() const { return values_.badge_menu_item_height_; }

double UiSizingSettings::getIconButtonSize() const { return values_.icon_button_size_; }

double UiSizingSettings::getIconGlyphSize() const { return values_.icon_glyph_size_; }

double UiSizingSettings::getTabHeight() const { return values_.tab_height_; }

double UiSizingSettings::getCheckboxHeight() const { return values_.checkbox_height_; }

double UiSizingSettings::getCheckboxIndicatorSize() const { return values_.checkbox_indicator_size_; }

double UiSizingSettings::getSpinboxButtonSize() const { return values_.spinbox_button_size_; }

double UiSizingSettings::getCalibrationFactor() const { return calibration_factor_; }

double UiSizingSettings::getMinInteractivePx() const { return min_interactive_px_; }

double UiSizingSettings::getMaxInteractivePx() const { return max_interactive_px_; }

double UiSizingSettings::px(double value, double pixel_density_px_per_mm) const
{
  if (value <= 0.0)
  {
    return 0.0;
  }

  const double density = effectivePixelDensityPxPerMm(pixel_density_px_per_mm);
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
    qWarning() << "[UiSizingSettings::initializeDefaults] YAML configuration not loaded, using constructor defaults";
    return;
  }

  for (const auto& mapping : FIELD_MAPPINGS)
  {
    const double raw = yaml_loader.getValue<double>(mapping.yaml_key_, mapping.default_value_);
    values_.*(mapping.member_) = sanitizePositive(raw, mapping.default_value_);
  }

  calibration_factor_ = sanitizePositive(yaml_loader.getValue<double>("ui.calibration_factor", DEFAULT_CALIBRATION_FACTOR), DEFAULT_CALIBRATION_FACTOR);

  min_interactive_px_ = sanitizePositive(yaml_loader.getValue<double>("ui.min_interactive_px", DEFAULT_MIN_INTERACTIVE_PX), DEFAULT_MIN_INTERACTIVE_PX);

  max_interactive_px_ = sanitizePositive(yaml_loader.getValue<double>("ui.max_interactive_px", DEFAULT_MAX_INTERACTIVE_PX), DEFAULT_MAX_INTERACTIVE_PX);

  if (max_interactive_px_ < min_interactive_px_)
  {
    qWarning() << "[UiSizingSettings::initializeDefaults] max_interactive_px < min_interactive_px, swapping";
    std::swap(max_interactive_px_, min_interactive_px_);
  }

  pixel_density_override_px_per_mm_ =
      std::max(0.0, yaml_loader.getValue<double>("ui.pixel_density_override_px_per_mm", DEFAULT_PIXEL_DENSITY_OVERRIDE_PX_PER_MM));

  min_pixel_density_px_per_mm_ = sanitizePositive(yaml_loader.getValue<double>("ui.min_pixel_density_px_per_mm", DEFAULT_MIN_PIXEL_DENSITY_PX_PER_MM),
                                                  DEFAULT_MIN_PIXEL_DENSITY_PX_PER_MM);

  max_pixel_density_px_per_mm_ = sanitizePositive(yaml_loader.getValue<double>("ui.max_pixel_density_px_per_mm", DEFAULT_MAX_PIXEL_DENSITY_PX_PER_MM),
                                                  DEFAULT_MAX_PIXEL_DENSITY_PX_PER_MM);

  if (max_pixel_density_px_per_mm_ < min_pixel_density_px_per_mm_)
  {
    qWarning() << "[UiSizingSettings::initializeDefaults] max_pixel_density_px_per_mm < min_pixel_density_px_per_mm, swapping";
    std::swap(max_pixel_density_px_per_mm_, min_pixel_density_px_per_mm_);
  }

  reference_diagonal_inch_ =
      sanitizePositive(yaml_loader.getValue<double>("ui.reference_diagonal_inch", DEFAULT_REFERENCE_DIAGONAL_INCH), DEFAULT_REFERENCE_DIAGONAL_INCH);

  min_scale_ = sanitizePositive(yaml_loader.getValue<double>("ui.min_scale", DEFAULT_MIN_SCALE), DEFAULT_MIN_SCALE);
  max_scale_ = sanitizePositive(yaml_loader.getValue<double>("ui.max_scale", DEFAULT_MAX_SCALE), DEFAULT_MAX_SCALE);

  if (max_scale_ < min_scale_)
  {
    qWarning() << "[UiSizingSettings::initializeDefaults] max_scale < min_scale, swapping";
    std::swap(max_scale_, min_scale_);
  }

  emit sizingChanged();
}

UiSizingSettings::UiSizingSettings() :
    calibration_factor_(DEFAULT_CALIBRATION_FACTOR),
    min_interactive_px_(DEFAULT_MIN_INTERACTIVE_PX),
    max_interactive_px_(DEFAULT_MAX_INTERACTIVE_PX),
    pixel_density_override_px_per_mm_(DEFAULT_PIXEL_DENSITY_OVERRIDE_PX_PER_MM),
    min_pixel_density_px_per_mm_(DEFAULT_MIN_PIXEL_DENSITY_PX_PER_MM),
    max_pixel_density_px_per_mm_(DEFAULT_MAX_PIXEL_DENSITY_PX_PER_MM),
    reference_diagonal_inch_(DEFAULT_REFERENCE_DIAGONAL_INCH),
    min_scale_(DEFAULT_MIN_SCALE),
    max_scale_(DEFAULT_MAX_SCALE)
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

double UiSizingSettings::effectivePixelDensityPxPerMm(double provided_density) const
{
  double density = provided_density;

  if (pixel_density_override_px_per_mm_ > 0.0)
  {
    density = pixel_density_override_px_per_mm_;
    return density;
  }
  else if (density <= 0.0)
  {
    density = fallbackPixelDensityPxPerMm();
  }

  if (min_pixel_density_px_per_mm_ > 0.0)
  {
    density = std::max(density, min_pixel_density_px_per_mm_);
  }
  if (max_pixel_density_px_per_mm_ > 0.0)
  {
    density = std::min(density, max_pixel_density_px_per_mm_);
  }

  const double diagonal_inch = screenDiagonalInches();
  if (diagonal_inch > 0.0 && reference_diagonal_inch_ > 0.0)
  {
    double scale = diagonal_inch / reference_diagonal_inch_;
    if (min_scale_ > 0.0)
    {
      scale = std::max(scale, min_scale_);
    }
    if (max_scale_ > 0.0)
    {
      scale = std::min(scale, max_scale_);
    }
    density *= scale;
  }

  return density;
}

double UiSizingSettings::screenDiagonalInches() const
{
  QScreen* screen = QGuiApplication::primaryScreen();
  if (!screen)
  {
    return 0.0;
  }

  const QSizeF size_mm = screen->physicalSize();
  if (size_mm.width() <= 0.0 || size_mm.height() <= 0.0)
  {
    return 0.0;
  }

  const double width_in = size_mm.width() / MILLIMETERS_PER_INCH;
  const double height_in = size_mm.height() / MILLIMETERS_PER_INCH;
  return std::sqrt(width_in * width_in + height_in * height_in);
}
