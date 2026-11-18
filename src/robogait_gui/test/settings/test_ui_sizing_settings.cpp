#include <filesystem>
#include <fstream>

#include <QObject>
#include <gtest/gtest.h>

#include "Loader/YamlLoader.hpp"
#include "Settings/UiSizingSettings.hpp"

TEST(UiSizingSettingsTest, LoadsEveryComponentAndConvertsDeterministically)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_ui_sizing_settings_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "ui:\n";
  config_file << "  button_height: 1.0\n";
  config_file << "  input_height: 2.0\n";
  config_file << "  combo_height: 3.0\n";
  config_file << "  dropdown_height: 4.0\n";
  config_file << "  list_item_height: 5.0\n";
  config_file << "  badge_height: 6.0\n";
  config_file << "  badge_menu_item_height: 7.0\n";
  config_file << "  icon_button_size: 8.0\n";
  config_file << "  icon_glyph_size: 9.0\n";
  config_file << "  tab_height: 10.0\n";
  config_file << "  checkbox_height: 11.0\n";
  config_file << "  checkbox_indicator_size: 12.0\n";
  config_file << "  spinbox_button_size: 13.0\n";
  config_file << "  joystick_stick_size: 14.0\n";
  config_file << "  pixel_density_override_px_per_mm: 4.0\n";
  config_file << "  min_pixel_density_px_per_mm: 2.5\n";
  config_file << "  max_pixel_density_px_per_mm: 5.5\n";
  config_file << "  reference_diagonal_inch: 13.0\n";
  config_file << "  min_scale: 0.85\n";
  config_file << "  max_scale: 1.15\n";
  config_file << "  calibration_factor: 2.0\n";
  config_file << "  min_interactive_px: 40.0\n";
  config_file << "  max_interactive_px: 100.0\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const auto load_result = yaml_loader.loadConfig(config_path.string());
  ASSERT_TRUE(static_cast<bool>(load_result)) << load_result.error;

  auto& settings = ROBOGait::settings::UiSizingSettings::getInstance();
  QObject connection_context;
  int sizing_changes = 0;

  // clang-format off
  QObject::connect(&settings,
                   &ROBOGait::settings::UiSizingSettings::sizingChanged,
                   &connection_context, [&sizing_changes]() { ++sizing_changes; });
  // clang-format on

  settings.initializeDefaults();

  EXPECT_EQ(sizing_changes, 1);
  EXPECT_DOUBLE_EQ(settings.getButtonHeight(), 1.0);
  EXPECT_DOUBLE_EQ(settings.getInputHeight(), 2.0);
  EXPECT_DOUBLE_EQ(settings.getComboHeight(), 3.0);
  EXPECT_DOUBLE_EQ(settings.getDropdownHeight(), 4.0);
  EXPECT_DOUBLE_EQ(settings.getListItemHeight(), 5.0);
  EXPECT_DOUBLE_EQ(settings.getBadgeHeight(), 6.0);
  EXPECT_DOUBLE_EQ(settings.getBadgeMenuItemHeight(), 7.0);
  EXPECT_DOUBLE_EQ(settings.getIconButtonSize(), 8.0);
  EXPECT_DOUBLE_EQ(settings.getIconGlyphSize(), 9.0);
  EXPECT_DOUBLE_EQ(settings.getTabHeight(), 10.0);
  EXPECT_DOUBLE_EQ(settings.getCheckboxHeight(), 11.0);
  EXPECT_DOUBLE_EQ(settings.getCheckboxIndicatorSize(), 12.0);
  EXPECT_DOUBLE_EQ(settings.getSpinboxButtonSize(), 13.0);
  EXPECT_DOUBLE_EQ(settings.getJoystickStickSize(), 14.0);
  EXPECT_DOUBLE_EQ(settings.getCalibrationFactor(), 2.0);
  EXPECT_DOUBLE_EQ(settings.getMinInteractivePx(), 40.0);
  EXPECT_DOUBLE_EQ(settings.getMaxInteractivePx(), 100.0);

  EXPECT_DOUBLE_EQ(settings.px(5.0, 50.0), 40.0);
  EXPECT_DOUBLE_EQ(settings.px(-1.0, 50.0), 0.0);
  EXPECT_DOUBLE_EQ(settings.interactivePx(1.0, 50.0), 40.0);
  EXPECT_DOUBLE_EQ(settings.interactivePx(20.0, 50.0), 100.0);

  std::filesystem::remove_all(test_dir);
}

TEST(UiSizingSettingsTest, SanitizesInvalidValuesAndOrdersInteractiveLimits)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_ui_sizing_settings_invalid_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "ui:\n";
  config_file << "  button_height: -1.0\n";
  config_file << "  calibration_factor: 0.0\n";
  config_file << "  min_interactive_px: 100.0\n";
  config_file << "  max_interactive_px: 40.0\n";
  config_file << "  pixel_density_override_px_per_mm: -2.0\n";
  config_file << "  min_pixel_density_px_per_mm: 6.0\n";
  config_file << "  max_pixel_density_px_per_mm: 3.0\n";
  config_file << "  reference_diagonal_inch: -1.0\n";
  config_file << "  min_scale: 1.2\n";
  config_file << "  max_scale: 0.8\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const auto load_result = yaml_loader.loadConfig(config_path.string());
  ASSERT_TRUE(static_cast<bool>(load_result)) << load_result.error;

  auto& settings = ROBOGait::settings::UiSizingSettings::getInstance();
  settings.initializeDefaults();

  EXPECT_DOUBLE_EQ(settings.getButtonHeight(), 12.0);
  EXPECT_DOUBLE_EQ(settings.getInputHeight(), 12.0);
  EXPECT_DOUBLE_EQ(settings.getCalibrationFactor(), 1.0);
  EXPECT_DOUBLE_EQ(settings.getMinInteractivePx(), 40.0);
  EXPECT_DOUBLE_EQ(settings.getMaxInteractivePx(), 100.0);

  std::filesystem::remove_all(test_dir);
}