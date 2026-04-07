#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "Functions.hpp"

class FunctionsTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Create a temporary directory for test files
    test_dir_ = std::filesystem::temp_directory_path() / "functions_test";
    std::filesystem::create_directory(test_dir_);

    // Create a dummy yaml file
    dummy_yaml_path_ = test_dir_ / "test_map.yaml";
    std::ofstream yaml_file(dummy_yaml_path_);
    yaml_file << "image: test_map.pgm\n";
    yaml_file << "resolution: 0.05\n";
    yaml_file << "origin: [-10.0, -10.0, 0.0]\n";
    yaml_file.close();

    // Create a dummy pgm file
    dummy_pgm_path_ = test_dir_ / "test_map.pgm";
    std::ofstream pgm_file(dummy_pgm_path_, std::ios::binary);
    pgm_file.write("P5\n2 2\n255\n", 11);
    pgm_file.write("\xFF\x80\x40\x00", 4);
    pgm_file.close();
  }

  void TearDown() override
  {
    // Clean up the temporary directory
    std::filesystem::remove_all(test_dir_);
  }

  std::filesystem::path test_dir_;
  std::filesystem::path dummy_yaml_path_;
  std::filesystem::path dummy_pgm_path_;
};

TEST_F(FunctionsTest, GetMapYamlInfoSuccess)
{
  auto content = ROBOGait::functions::getMapYamlInfo(test_dir_.string(), "test_map");
  ASSERT_TRUE(content.has_value());
  EXPECT_TRUE(content.value().find("image: test_map.pgm") != std::string::npos);
  EXPECT_TRUE(content.value().find("resolution: 0.05") != std::string::npos);
}

TEST_F(FunctionsTest, GetMapYamlInfoFailure)
{
  auto content = ROBOGait::functions::getMapYamlInfo(test_dir_.string(), "non_existent_map");
  EXPECT_FALSE(content.has_value());
}

TEST_F(FunctionsTest, GetMapPgmInfoSuccess)
{
  auto content = ROBOGait::functions::getMapPgmInfo(test_dir_.string(), "test_map");
  ASSERT_TRUE(content.has_value());
  EXPECT_EQ(content.value().size(), 15);
}

TEST_F(FunctionsTest, GetMapPgmInfoFailure)
{
  auto content = ROBOGait::functions::getMapPgmInfo(test_dir_.string(), "non_existent_map");
  EXPECT_FALSE(content.has_value());
}

TEST_F(FunctionsTest, ExpandPathTilde)
{
  std::string home = std::getenv("HOME");
  std::string path = "~/test/path";
  std::string expanded = ROBOGait::functions::expandPath(path);
  EXPECT_EQ(expanded, home + "/test/path");
}

TEST_F(FunctionsTest, ExpandPathHome)
{
  std::string home = std::getenv("HOME");
  std::string path = "$HOME/test/path";
  std::string expanded = ROBOGait::functions::expandPath(path);
  EXPECT_EQ(expanded, home + "/test/path");
}

TEST_F(FunctionsTest, ExpandPathNoExpansion)
{
  std::string path = "/absolute/path";
  std::string expanded = ROBOGait::functions::expandPath(path);
  EXPECT_EQ(expanded, path);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
