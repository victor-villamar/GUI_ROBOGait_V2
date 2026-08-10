#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "Loader/BootStrapLoader.hpp"

namespace
{

class BootStrapLoaderTest : public testing::Test
{
protected:
  void SetUp() override
  {
    test_dir_ = std::filesystem::temp_directory_path() / "robogait_gui_bootstrap_loader_test";
    std::filesystem::remove_all(test_dir_);
    std::filesystem::create_directories(test_dir_);
  }

  void TearDown() override { std::filesystem::remove_all(test_dir_); }

  std::filesystem::path writeYaml(const std::string& filename, const std::string& content) const
  {
    const std::filesystem::path yaml_path = test_dir_ / filename;
    std::ofstream yaml_file(yaml_path);
    yaml_file << content;
    yaml_file.close();

    return yaml_path;
  }

  std::filesystem::path test_dir_;
};

} // namespace

TEST_F(BootStrapLoaderTest, LoadsValidBootstrapConfig)
{
  const std::filesystem::path yaml_path = writeYaml("bootstrap.yaml", "user_config_root: \".local/robogait/params\"\n"
                                                                      "config_file: \"config.yaml\"\n"
                                                                      "commands_file: \"commands.yaml\"\n");

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();

  ASSERT_TRUE(bootstrap_loader.loadBootStrapConfig(yaml_path));

  const auto config = bootstrap_loader.getBootStrapConfig();
  EXPECT_EQ(config.user_config_root_path, ".local/robogait/params");
  EXPECT_EQ(config.config_file_name, "config.yaml");
  EXPECT_EQ(config.commands_file_name, "commands.yaml");
}

TEST_F(BootStrapLoaderTest, ReturnsFalseWhenFileDoesNotExist)
{
  const std::filesystem::path missing_path = test_dir_ / "missing.yaml";

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();

  EXPECT_FALSE(bootstrap_loader.loadBootStrapConfig(missing_path));
}

TEST_F(BootStrapLoaderTest, ReturnsFalseWhenYamlIsMalformed)
{
  const std::filesystem::path yaml_path = writeYaml("malformed.yaml", "user_config_root: [\n"
                                                                      "config_file: \"config.yaml\"\n"
                                                                      "commands_file: \"commands.yaml\"\n");

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();

  EXPECT_FALSE(bootstrap_loader.loadBootStrapConfig(yaml_path));
}

TEST_F(BootStrapLoaderTest, ReturnsFalseWhenRequiredKeyIsMissing)
{
  const std::filesystem::path yaml_path = writeYaml("missing_key.yaml", "user_config_root: \".local/robogait/params\"\n"
                                                                        "config_file: \"config.yaml\"\n");

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();

  EXPECT_FALSE(bootstrap_loader.loadBootStrapConfig(yaml_path));
}

TEST_F(BootStrapLoaderTest, ReturnsFalseWhenRequiredValueHasInvalidType)
{
  const std::filesystem::path yaml_path = writeYaml("invalid_type.yaml", "user_config_root: \".local/robogait/params\"\n"
                                                                         "config_file:\n"
                                                                         "  name: \"config.yaml\"\n"
                                                                         "commands_file: \"commands.yaml\"\n");

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();

  EXPECT_FALSE(bootstrap_loader.loadBootStrapConfig(yaml_path));
}

TEST_F(BootStrapLoaderTest, ReturnsFalseWhenRequiredValueIsEmpty)
{
  const std::filesystem::path yaml_path = writeYaml("empty_value.yaml", "user_config_root: \".local/robogait/params\"\n"
                                                                        "config_file: \"\"\n"
                                                                        "commands_file: \"commands.yaml\"\n");

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();

  EXPECT_FALSE(bootstrap_loader.loadBootStrapConfig(yaml_path));
}
