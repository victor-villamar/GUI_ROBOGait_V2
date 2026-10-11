#include <filesystem>
#include <fstream>

#include <QObject>
#include <gtest/gtest.h>

#include "Loader/YamlLoader.hpp"
#include "Robot/ManualControl.hpp"
#include "Settings/RobotSettings.hpp"

TEST(ManualControlSettingsTest, AppliesRuntimeVelocityLimitsAndClampsCurrentCommand)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_manual_control_settings_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "robot:\n";
  config_file << "  radius: 0.326\n";
  config_file << "  max_linear_velocity: 0.22\n";
  config_file << "  max_angular_velocity: 2.84\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const auto load_result = yaml_loader.loadConfig(config_path.string());
  ASSERT_TRUE(static_cast<bool>(load_result)) << load_result.error;

  auto& robot_settings = ROBOGait::settings::RobotSettings::getInstance();
  robot_settings.initializeDefaults();

  ROBOGait::robot::control::ManualControl manual_control;
  EXPECT_DOUBLE_EQ(manual_control.getMaxLinearVelocity(), 0.22);
  EXPECT_DOUBLE_EQ(manual_control.getMaxAngularVelocity(), 2.84);

  manual_control.updateVelocity(0.2, 2.0);

  QObject connection_context;
  int limit_changes = 0;
  int velocity_changes = 0;

  // clang-format off
  QObject::connect(&manual_control,
                   &ROBOGait::robot::control::ManualControl::velocityLimitsChanged,
                   &connection_context,
                   [&limit_changes]() { ++limit_changes; });
  QObject::connect(&manual_control,
                   &ROBOGait::robot::control::ManualControl::velocityChanged,
                   &connection_context,
                   [&velocity_changes]() { ++velocity_changes; });
  // clang-format on

  robot_settings.setMaxLinearVelocity(0.1);
  robot_settings.setMaxAngularVelocity(0.5);

  EXPECT_DOUBLE_EQ(manual_control.getMaxLinearVelocity(), 0.1);
  EXPECT_DOUBLE_EQ(manual_control.getMaxAngularVelocity(), 0.5);
  EXPECT_DOUBLE_EQ(manual_control.getLinearVelocity(), 0.1);
  EXPECT_DOUBLE_EQ(manual_control.getAngularVelocity(), 0.5);
  EXPECT_EQ(limit_changes, 2);
  EXPECT_EQ(velocity_changes, 2);

  std::filesystem::remove_all(test_dir);
}