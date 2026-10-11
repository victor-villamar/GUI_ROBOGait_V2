#include <filesystem>
#include <fstream>

#include <QObject>
#include <gtest/gtest.h>

#include "Loader/YamlLoader.hpp"
#include "Settings/TimeoutSettings.hpp"

TEST(TimeoutSettingsTest, LoadsValuesAndFallsBackForNonPositiveTimeouts)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_timeout_settings_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "timeouts:\n";
  config_file << "  qml:\n";
  config_file << "    app_shutdown_delay_ms: 3100\n";
  config_file << "    auth_mode_dropdown_auto_close_ms: 4100\n";
  config_file << "    register_default_idle_timeout_ms: 11000\n";
  config_file << "    register_sign_in_idle_timeout_ms: 61000\n";
  config_file << "    badge_auto_close_ms: 3200\n";
  config_file << "    robot_badge_auto_close_ms: 6200\n";
  config_file << "    error_rectangle_delayed_close_ms: 5200\n";
  config_file << "    test_busy_timeout_ms: 12000\n";
  config_file << "    auto_localization_spin_timeout_ms: 31000\n";
  config_file << "    auto_localization_spin_publish_period_ms: 0\n";
  config_file << "    experiment_save_prompt_delay_ms: -1\n";
  config_file << "    button_hold_ms: 700\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const auto load_result = yaml_loader.loadConfig(config_path.string());
  ASSERT_TRUE(static_cast<bool>(load_result)) << load_result.error;

  auto& settings = ROBOGait::settings::TimeoutSettings::getInstance();
  QObject connection_context;
  int timeout_changes = 0;

  // clang-format off
  QObject::connect(&settings,
                   &ROBOGait::settings::TimeoutSettings::timeoutsChanged,
                   &connection_context, [&timeout_changes]() { ++timeout_changes; });
  // clang-format on

  settings.initializeDefaults();

  EXPECT_EQ(timeout_changes, 1);
  EXPECT_EQ(settings.getAppShutdownDelayMs(), 3100);
  EXPECT_EQ(settings.getAuthModeDropdownAutoCloseMs(), 4100);
  EXPECT_EQ(settings.getRegisterDefaultIdleTimeoutMs(), 11000);
  EXPECT_EQ(settings.getRegisterSignInIdleTimeoutMs(), 61000);
  EXPECT_EQ(settings.getBadgeAutoCloseMs(), 3200);
  EXPECT_EQ(settings.getRobotBadgeAutoCloseMs(), 6200);
  EXPECT_EQ(settings.getErrorRectangleDelayedCloseMs(), 5200);
  EXPECT_EQ(settings.getTestBusyTimeoutMs(), 12000);
  EXPECT_EQ(settings.getAutoLocalizationSpinTimeoutMs(), 31000);
  EXPECT_EQ(settings.getAutoLocalizationSpinPublishPeriodMs(), 100);
  EXPECT_EQ(settings.getExperimentSavePromptDelayMs(), 3000);
  EXPECT_EQ(settings.getButtonHoldMs(), 700);

  std::filesystem::remove_all(test_dir);
}