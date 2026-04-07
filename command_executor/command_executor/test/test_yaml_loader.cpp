#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "YamlLoader.hpp"

class YamlLoaderTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    test_dir_ = std::filesystem::temp_directory_path() / "yaml_loader_test";
    std::filesystem::create_directory(test_dir_);

    dummy_yaml_path_ = test_dir_ / "config.yaml";
    std::ofstream yaml_file(dummy_yaml_path_);
    yaml_file << "robot:\n";
    yaml_file << "  name: 'robogait'\n";
    yaml_file << "  ip: '127.0.0.1'\n";
    yaml_file << "parameters:\n";
    yaml_file << "  speed: 1.2\n";
    yaml_file.close();
  }

  void TearDown() override { std::filesystem::remove_all(test_dir_); }

  std::filesystem::path test_dir_;
  std::filesystem::path dummy_yaml_path_;
};

TEST_F(YamlLoaderTest, LoadConfigSuccess)
{
  auto& loader = ROBOGait::loader::YamlLoader::getInstance();
  ASSERT_TRUE(loader.loadConfig(dummy_yaml_path_.string()));
  EXPECT_TRUE(loader.isLoaded());
}

TEST_F(YamlLoaderTest, LoadConfigFailure)
{
  auto& loader = ROBOGait::loader::YamlLoader::getInstance();
  ASSERT_FALSE(loader.loadConfig("non_existent_file.yaml"));
  EXPECT_FALSE(loader.isLoaded());
}

TEST_F(YamlLoaderTest, GetValueSuccess)
{
  auto& loader = ROBOGait::loader::YamlLoader::getInstance();
  loader.loadConfig(dummy_yaml_path_.string());

  std::string robot_name = loader.getValue<std::string>("robot.name", "");
  EXPECT_EQ(robot_name, "robogait");

  double speed = loader.getValue<double>("parameters.speed", 0.0);
  EXPECT_EQ(speed, 1.2);
}

TEST_F(YamlLoaderTest, GetValueFailure)
{
  auto& loader = ROBOGait::loader::YamlLoader::getInstance();
  loader.loadConfig(dummy_yaml_path_.string());

  std::string non_existent = loader.getValue<std::string>("robot.non_existent_key", "default");
  EXPECT_EQ(non_existent, "default");
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
