#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "Loader/YamlLoader.hpp"

TEST(YamlLoaderTest, LoadsNestedValuesAndReturnsDefaults)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_yaml_loader_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "database:\n";
  config_file << "  path: \"/tmp/robogait.db\"\n";
  config_file << "map:\n";
  config_file << "  spline_path:\n";
  config_file << "    follow_path_min_point_spacing_m: 0.25\n";
  config_file << "features:\n";
  config_file << "  enabled: true\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  ASSERT_TRUE(yaml_loader.loadConfig(config_path.string()));
  EXPECT_TRUE(yaml_loader.isLoaded());
  EXPECT_EQ(yaml_loader.getValue<std::string>("database.path", "default"), "/tmp/robogait.db");
  EXPECT_DOUBLE_EQ(yaml_loader.getValue<double>("map.spline_path.follow_path_min_point_spacing_m", 0.0), 0.25);
  EXPECT_TRUE(yaml_loader.getValue<bool>("features.enabled", false));
  EXPECT_EQ(yaml_loader.getValue<int>("missing.value", 42), 42);
  EXPECT_EQ(yaml_loader.getValue<int>("database.path", 42), 42);

  const std::filesystem::path missing_path = test_dir / "missing.yaml";
  EXPECT_FALSE(yaml_loader.loadConfig(missing_path.string()));
  EXPECT_FALSE(yaml_loader.isLoaded());
  EXPECT_EQ(yaml_loader.getValue<int>("database.path", 42), 42);

  std::filesystem::remove_all(test_dir);
}