#pragma once

#include <QObject>

namespace ROBOGait
{
namespace settings
{
/**
 * @brief Singleton class to manage UI sizing settings
 *
 * This class provides a centralized way to manage UI sizing settings, allowing for consistent scaling across the application.
 */
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
  Q_PROPERTY(double buttonHeight
           READ getButtonHeight
           NOTIFY sizingChanged)
  
  Q_PROPERTY(double inputHeight
             READ getInputHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double comboHeight
             READ getComboHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double dropdownHeight
           READ getDropdownHeight
           NOTIFY sizingChanged)

  Q_PROPERTY(double listItemHeight
             READ getListItemHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double badgeHeight
             READ getBadgeHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double badgeMenuItemHeight
             READ getBadgeMenuItemHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double iconButtonSize
             READ getIconButtonSize
             NOTIFY sizingChanged)

  Q_PROPERTY(double iconGlyphSize
             READ getIconGlyphSize
             NOTIFY sizingChanged)

  Q_PROPERTY(double tabHeight
             READ getTabHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double checkboxHeight
             READ getCheckboxHeight
             NOTIFY sizingChanged)

  Q_PROPERTY(double checkboxIndicatorSize
             READ getCheckboxIndicatorSize
             NOTIFY sizingChanged)

  Q_PROPERTY(double spinboxButtonSize
             READ getSpinboxButtonSize
             NOTIFY sizingChanged)

  Q_PROPERTY(double joystickStickSize
             READ getJoystickStickSize
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

  /**
   * @brief Get the height for button UI elements
   *
   * @return Height in millimeters
   */
  double getButtonHeight() const;

  /**
   * @brief Get the height for input UI elements
   *
   * @return Height in millimeters
   */
  double getInputHeight() const;

  /**
   * @brief Get the height for combo box UI elements
   *
   * @return Height in millimeters
   */
  double getComboHeight() const;

  /**
   * @brief Get the height for dropdown UI elements
   *
   * @return Height in millimeters
   */
  double getDropdownHeight() const;

  /**
   * @brief Get the height for list item UI elements
   *
   * @return Height in millimeters
   */
  double getListItemHeight() const;

  /**
   * @brief Get the height for badge UI elements
   *
   * @return Height in millimeters
   */
  double getBadgeHeight() const;

  /**
   * @brief Get the height for badge menu item UI elements
   *
   * @return Height in millimeters
   */
  double getBadgeMenuItemHeight() const;

  /**
   * @brief Get the size for icon button UI elements
   *
   * @return Size in millimeters
   */
  double getIconButtonSize() const;

  /**
   * @brief Get the size for icon glyphs
   *
   * @return Size in millimeters
   */
  double getIconGlyphSize() const;

  /**
   * @brief Get the height for tab UI elements
   *
   * @return Height in millimeters
   */
  double getTabHeight() const;

  /**
   * @brief Get the height for checkbox UI elements
   *
   * @return Height in millimeters
   */
  double getCheckboxHeight() const;

  /**
   * @brief Get the size for checkbox indicators
   *
   * @return Size in millimeters
   */
  double getCheckboxIndicatorSize() const;

  /**
   * @brief Get the size for spinbox buttons
   *
   * @return Size in millimeters
   */
  double getSpinboxButtonSize() const;

  /**
   * @brief Get the size for joystick stick
   *
   * @return Size in millimeters
   */
  double getJoystickStickSize() const;

  /**
   * @brief Get the calibration factor
   *
   * @return Calibration factor
   */
  double getCalibrationFactor() const;

  /**
   * @brief Get the minimum interactive pixel value
   *
   * @return Minimum interactive pixel value
   */
  double getMinInteractivePx() const;

  /**
   * @brief Get the maximum interactive pixel value
   *
   * @return Maximum interactive pixel value
   */
  double getMaxInteractivePx() const;

  /**
   * @brief Convert a value in millimeters to pixels based on the provided pixel density
   *
   * @param value The value in millimeters
   * @param pixel_density_px_per_mm The pixel density in pixels per millimeter
   *
   * @return The value in pixels
   */
  Q_INVOKABLE double px(double value, double pixel_density_px_per_mm) const;

  /**
   * @brief Convert a value in pixels to millimeters based on the provided pixel density
   *
   * @param value The value in pixels
   * @param pixel_density_px_per_mm The pixel density in pixels per millimeter
   *
   * @return The value in millimeters
   */
  Q_INVOKABLE double interactivePx(double value, double pixel_density_px_per_mm) const;

  /**
   * @brief Initialize default values
   */
  void initializeDefaults();

signals:
  void sizingChanged(); // Signal emitted when any sizing setting changes

public:
  /**
   * @brief Struct to hold UI sizing values, used for loading from YAML configuration
   *
   * @param button_height_ The height for button UI elements
   * @param input_height_ The height for input UI elements
   * @param combo_height_ The height for combo box UI elements
   * @param dropdown_height_ The height for dropdown UI elements
   * @param list_item_height_ The height for list item UI elements
   * @param badge_height_ The height for badge UI elements
   * @param badge_menu_item_height_ The height for badge menu item UI elements
   * @param icon_button_size_ The size for icon button UI elements
   * @param icon_glyph_size_ The size for icon glyphs
   * @param tab_height_ The height for tab UI elements
   * @param checkbox_height_ The height for checkbox UI elements
   * @param checkbox_indicator_size_ The size for checkbox indicators
   * @param spinbox_button_size_ The size for spinbox buttons
   * @param joystick_stick_size_ The size for joystick stick
   */
  struct UiSizingValues
  {
    double button_height_ = DEFAULT_COMPONENT_HEIGHT;
    double input_height_ = DEFAULT_COMPONENT_HEIGHT;
    double combo_height_ = DEFAULT_COMPONENT_HEIGHT;
    double dropdown_height_ = DEFAULT_COMPONENT_HEIGHT;
    double list_item_height_ = DEFAULT_COMPONENT_HEIGHT;
    double badge_height_ = DEFAULT_COMPONENT_HEIGHT;
    double badge_menu_item_height_ = DEFAULT_COMPONENT_HEIGHT;
    double icon_button_size_ = DEFAULT_COMPONENT_HEIGHT;
    double icon_glyph_size_ = DEFAULT_COMPONENT_HEIGHT;
    double tab_height_ = DEFAULT_COMPONENT_HEIGHT;
    double checkbox_height_ = DEFAULT_COMPONENT_HEIGHT;
    double checkbox_indicator_size_ = DEFAULT_COMPONENT_HEIGHT;
    double spinbox_button_size_ = DEFAULT_COMPONENT_HEIGHT;
    double joystick_stick_size_ = DEFAULT_COMPONENT_HEIGHT;
  };

  static constexpr double DEFAULT_COMPONENT_HEIGHT = 12.0; /**< Default component height in millimeters */

private:
  /**
   * @brief Constructor for UiSizingSettings class
   */
  UiSizingSettings();

  /**
   * @brief Destructor for UiSizingSettings class
   */
  ~UiSizingSettings() override = default;

  /**
   * @brief Sanitize a value to ensure it is positive, returning a fallback if it is not
   *
   * @param value The value to sanitize
   * @param fallback The fallback value to return if the input value is not positive
   *
   * @return The sanitized value or the fallback if the input value is not positive
   */
  double sanitizePositive(double value, double fallback) const;

  /**
   * @brief Get the fallback pixel density in pixels per millimeter
   *
   * @return The fallback pixel density
   */
  double fallbackPixelDensityPxPerMm() const;

  /**
   * @brief Get the effective pixel density in pixels per millimeter
   *
   * @param provided_density Optional provided density from UI (<=0 uses fallback)
   *
   * @return The effective pixel density after override and clamping
   */
  double effectivePixelDensityPxPerMm(double provided_density) const;

  /**
   * @brief Get the physical screen diagonal in inches
   *
   * @return Screen diagonal in inches, or 0 if unavailable
   */
  double screenDiagonalInches() const;

  UiSizingValues values_;                   /**< Struct to hold UI sizing values loaded from YAML configuration */
  double calibration_factor_;               /**< The calibration factor for scaling UI elements */
  double min_interactive_px_;               /**< The minimum interactive pixel value */
  double max_interactive_px_;               /**< The maximum interactive pixel value */
  double pixel_density_override_px_per_mm_; /**< Override pixel density in px/mm (0 = auto) */
  double min_pixel_density_px_per_mm_;      /**< Minimum pixel density clamp in px/mm */
  double max_pixel_density_px_per_mm_;      /**< Maximum pixel density clamp in px/mm */
  double reference_diagonal_inch_;          /**< Reference screen diagonal in inches for scaling */
  double min_scale_;                        /**< Minimum scale factor for screen size */
  double max_scale_;                        /**< Maximum scale factor for screen size */

  static constexpr double DEFAULT_CALIBRATION_FACTOR = 1.0;               /**< Default calibration factor for scaling UI elements */
  static constexpr double DEFAULT_MIN_INTERACTIVE_PX = 44.0;              /**< Default minimum interactive pixel value */
  static constexpr double DEFAULT_MAX_INTERACTIVE_PX = 96.0;              /**< Default maximum interactive pixel value */
  static constexpr double DEFAULT_PIXEL_DENSITY_OVERRIDE_PX_PER_MM = 0.0; /**< Default pixel density override (0 = auto) */
  static constexpr double DEFAULT_MIN_PIXEL_DENSITY_PX_PER_MM = 2.5;      /**< Default minimum pixel density clamp */
  static constexpr double DEFAULT_MAX_PIXEL_DENSITY_PX_PER_MM = 5.5;      /**< Default maximum pixel density clamp */
  static constexpr double DEFAULT_REFERENCE_DIAGONAL_INCH = 13.0;         /**< Default reference screen diagonal in inches */
  static constexpr double DEFAULT_MIN_SCALE = 0.85;                       /**< Default minimum scale factor */
  static constexpr double DEFAULT_MAX_SCALE = 1.15;                       /**< Default maximum scale factor */

  static constexpr double DEFAULT_FALLBACK_DENSITY_PX_PER_MM = 3.779527559; /**< Default fallback pixel density in pixels per millimeter (96 DPI) */
  static constexpr double MILLIMETERS_PER_INCH = 25.4;                      /**< Number of millimeters in an inch, used for pixel density calculations */
};
} // namespace settings
} // namespace ROBOGait
