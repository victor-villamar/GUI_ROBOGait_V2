#include <filesystem>
#include <fstream>
#include <limits>

#include <QObject>
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "Loader/YamlLoader.hpp"
#include "Settings/RobotSettings.hpp"

TEST(RobotSettingsTest, LoadsValidatesUpdatesInMemoryAndNotifies)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_robot_settings_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "robot:\n";
  config_file << "  radius: 0.326\n";
  config_file << "  max_linear_velocity: 0.22\n";
  config_file << "  max_angular_velocity: 2.84\n";
  config_file << "topics:\n";
  config_file << "  cmd_vel: /cmd_vel\n";
  config_file << "  scan: /scan\n";
  config_file << "  camera_detection: /camera_detection\n";
  config_file << "frames:\n";
  config_file << "  map: map\n";
  config_file << "  robot: base_link\n";
  config_file << "map:\n";
  config_file << "  image_path: .local/robogait/maps/\n";
  config_file << "  map_path: /maps/\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const auto load_result = yaml_loader.loadConfig(config_path.string());
  ASSERT_TRUE(static_cast<bool>(load_result)) << load_result.error;

  auto& settings = ROBOGait::settings::RobotSettings::getInstance();
  settings.initializeDefaults();

  EXPECT_DOUBLE_EQ(settings.getRobotRadius(), 0.326);
  EXPECT_DOUBLE_EQ(settings.getMaxLinearVelocity(), 0.22);
  EXPECT_DOUBLE_EQ(settings.getMaxAngularVelocity(), 2.84);
  EXPECT_DOUBLE_EQ(settings.getMaxAllowedLinearVelocity(), 1.0);
  EXPECT_DOUBLE_EQ(settings.getMaxAllowedAngularVelocity(), 4.0);
  EXPECT_EQ(settings.getCmdVelTopic(), QStringLiteral("/cmd_vel"));
  EXPECT_EQ(settings.getScanTopic(), QStringLiteral("/scan"));
  EXPECT_EQ(settings.getCameraDetectionTopic(), QStringLiteral("/camera_detection"));
  EXPECT_EQ(settings.getMapFrame(), QStringLiteral("map"));
  EXPECT_EQ(settings.getRobotFrame(), QStringLiteral("base_link"));
  EXPECT_EQ(settings.getMapImagePath(), QStringLiteral(".local/robogait/maps/"));
  EXPECT_EQ(settings.getRobotMapPath(), QStringLiteral("/maps/"));

  QObject connection_context;
  int radius_changes = 0;
  int linear_velocity_changes = 0;
  int angular_velocity_changes = 0;
  int scan_topic_changes = 0;
  int robot_frame_changes = 0;

  // clang-format off
  QObject::connect(&settings,
                   &ROBOGait::settings::RobotSettings::robotRadiusChanged,
                   &connection_context, [&radius_changes]() { ++radius_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::RobotSettings::maxLinearVelocityChanged,
                   &connection_context, [&linear_velocity_changes]() { ++linear_velocity_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::RobotSettings::maxAngularVelocityChanged,
                   &connection_context, [&angular_velocity_changes]() { ++angular_velocity_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::RobotSettings::scanTopicChanged,
                   &connection_context, [&scan_topic_changes]() { ++scan_topic_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::RobotSettings::robotFrameChanged,
                   &connection_context, [&robot_frame_changes]() { ++robot_frame_changes; });
  // clang-format on

  settings.setRobotRadius(0.4);
  settings.setMaxLinearVelocity(0.35);
  settings.setMaxAngularVelocity(1.5);
  settings.setCmdVelTopic(QStringLiteral("/robot/cmd_vel"));
  settings.setScanTopic(QStringLiteral(" /front_scan "));
  settings.setCameraDetectionTopic(QStringLiteral("/perception/people"));
  settings.setMapFrame(QStringLiteral("world"));
  settings.setRobotFrame(QStringLiteral("robot/base_footprint"));

  EXPECT_EQ(radius_changes, 1);
  EXPECT_EQ(linear_velocity_changes, 1);
  EXPECT_EQ(angular_velocity_changes, 1);
  EXPECT_EQ(scan_topic_changes, 1);
  EXPECT_EQ(robot_frame_changes, 1);
  EXPECT_DOUBLE_EQ(settings.getRobotRadius(), 0.4);
  EXPECT_DOUBLE_EQ(settings.getMaxLinearVelocity(), 0.35);
  EXPECT_DOUBLE_EQ(settings.getMaxAngularVelocity(), 1.5);
  EXPECT_EQ(settings.getScanTopic(), QStringLiteral("/front_scan"));
  EXPECT_EQ(settings.getRobotFrame(), QStringLiteral("robot/base_footprint"));

  settings.setRobotRadius(-1.0);
  settings.setMaxLinearVelocity(std::numeric_limits<double>::infinity());
  settings.setMaxAngularVelocity(std::numeric_limits<double>::infinity());
  settings.setScanTopic(QStringLiteral("/invalid topic"));
  settings.setRobotFrame(QStringLiteral("/base_link"));

  EXPECT_EQ(radius_changes, 1);
  EXPECT_EQ(linear_velocity_changes, 1);
  EXPECT_EQ(angular_velocity_changes, 1);
  EXPECT_EQ(scan_topic_changes, 1);
  EXPECT_EQ(robot_frame_changes, 1);
  EXPECT_DOUBLE_EQ(settings.getRobotRadius(), 0.4);
  EXPECT_DOUBLE_EQ(settings.getMaxLinearVelocity(), 0.35);
  EXPECT_DOUBLE_EQ(settings.getMaxAngularVelocity(), 1.5);
  EXPECT_EQ(settings.getScanTopic(), QStringLiteral("/front_scan"));
  EXPECT_EQ(settings.getRobotFrame(), QStringLiteral("robot/base_footprint"));

  settings.setMaxLinearVelocity(1.0);
  settings.setMaxAngularVelocity(4.0);

  EXPECT_EQ(linear_velocity_changes, 2);
  EXPECT_EQ(angular_velocity_changes, 2);
  EXPECT_DOUBLE_EQ(settings.getMaxLinearVelocity(), 1.0);
  EXPECT_DOUBLE_EQ(settings.getMaxAngularVelocity(), 4.0);

  settings.setMaxLinearVelocity(1.01);
  settings.setMaxAngularVelocity(4.01);

  EXPECT_EQ(linear_velocity_changes, 2);
  EXPECT_EQ(angular_velocity_changes, 2);
  EXPECT_DOUBLE_EQ(settings.getMaxLinearVelocity(), 1.0);
  EXPECT_DOUBLE_EQ(settings.getMaxAngularVelocity(), 4.0);

  const YAML::Node unchanged_config = YAML::LoadFile(config_path.string());
  EXPECT_DOUBLE_EQ(unchanged_config["robot"]["radius"].as<double>(), 0.326);
  EXPECT_DOUBLE_EQ(unchanged_config["robot"]["max_linear_velocity"].as<double>(), 0.22);
  EXPECT_DOUBLE_EQ(unchanged_config["robot"]["max_angular_velocity"].as<double>(), 2.84);
  EXPECT_EQ(unchanged_config["topics"]["cmd_vel"].as<std::string>(), "/cmd_vel");
  EXPECT_EQ(unchanged_config["topics"]["scan"].as<std::string>(), "/scan");
  EXPECT_EQ(unchanged_config["topics"]["camera_detection"].as<std::string>(), "/camera_detection");
  EXPECT_EQ(unchanged_config["frames"]["map"].as<std::string>(), "map");
  EXPECT_EQ(unchanged_config["frames"]["robot"].as<std::string>(), "base_link");
  EXPECT_EQ(unchanged_config["map"]["image_path"].as<std::string>(), ".local/robogait/maps/");
  EXPECT_EQ(unchanged_config["map"]["map_path"].as<std::string>(), "/maps/");

  EXPECT_DOUBLE_EQ(yaml_loader.getValue<double>("robot.radius", 0.0), 0.326);
  EXPECT_EQ(yaml_loader.getValue<std::string>("topics.scan", ""), "/scan");

  std::filesystem::remove_all(test_dir);
}