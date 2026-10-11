#include <filesystem>
#include <fstream>

#include <QObject>
#include <gtest/gtest.h>

#include "Loader/YamlLoader.hpp"
#include "Settings/DeveloperSettings.hpp"

TEST(DeveloperSettingsTest, LoadsTracksAppliesAndResetsPendingChanges)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "robogait_gui_developer_settings_test";
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "ros2:\n";
  config_file << "  domain_id: 42\n";
  config_file << "  use_namespace_discovery: true\n";
  config_file << "  use_topic_filter: false\n";
  config_file.close();

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const auto load_result = yaml_loader.loadConfig(config_path.string());
  ASSERT_TRUE(static_cast<bool>(load_result)) << load_result.error;

  auto& settings = ROBOGait::settings::DeveloperSettings::getInstance();
  settings.initializeDefaults();

  EXPECT_TRUE(settings.getUseNamespaceDiscovery());
  EXPECT_EQ(settings.getRosDomainId(), 42);
  EXPECT_FALSE(settings.getUseTopicFilter());
  EXPECT_FALSE(settings.getHasPendingChanges());

  QObject connection_context;
  int namespace_changes = 0;
  int domain_changes = 0;
  int topic_filter_changes = 0;
  int pending_changes = 0;
  int applied_changes = 0;
  int reset_changes = 0;

  // clang-format off
  QObject::connect(&settings,
                   &ROBOGait::settings::DeveloperSettings::useNamespaceDiscoveryChanged,
                   &connection_context,
                   [&namespace_changes]() { ++namespace_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::DeveloperSettings::rosDomainIdChanged,
                   &connection_context, [&domain_changes]() { ++domain_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::DeveloperSettings::useTopicFilterChanged,
                   &connection_context,
                   [&topic_filter_changes]() { ++topic_filter_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::DeveloperSettings::hasPendingChangesChanged,
                   &connection_context,
                   [&pending_changes]() { ++pending_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::DeveloperSettings::settingsApplied,
                   &connection_context, [&applied_changes]() { ++applied_changes; });
  QObject::connect(&settings,
                   &ROBOGait::settings::DeveloperSettings::settingsReset,
                   &connection_context, [&reset_changes]() { ++reset_changes; });
  // clang-format on

  settings.setUseNamespaceDiscovery(false);
  settings.setRosDomainId(43);
  settings.setUseTopicFilter(true);

  EXPECT_FALSE(settings.getUseNamespaceDiscovery());
  EXPECT_EQ(settings.getRosDomainId(), 43);
  EXPECT_TRUE(settings.getUseTopicFilter());
  EXPECT_TRUE(settings.getHasPendingChanges());
  EXPECT_EQ(namespace_changes, 1);
  EXPECT_EQ(domain_changes, 1);
  EXPECT_EQ(topic_filter_changes, 1);
  EXPECT_EQ(pending_changes, 1);

  settings.resetChanges();

  EXPECT_TRUE(settings.getUseNamespaceDiscovery());
  EXPECT_EQ(settings.getRosDomainId(), 42);
  EXPECT_FALSE(settings.getUseTopicFilter());
  EXPECT_FALSE(settings.getHasPendingChanges());
  EXPECT_EQ(namespace_changes, 2);
  EXPECT_EQ(domain_changes, 2);
  EXPECT_EQ(topic_filter_changes, 2);
  EXPECT_EQ(pending_changes, 2);
  EXPECT_EQ(reset_changes, 1);

  settings.setRosDomainId(-1);
  settings.setRosDomainId(233);
  EXPECT_EQ(settings.getRosDomainId(), 42);
  EXPECT_EQ(domain_changes, 2);
  EXPECT_FALSE(settings.getHasPendingChanges());

  settings.setRosDomainId(7);
  EXPECT_TRUE(settings.getHasPendingChanges());
  EXPECT_TRUE(settings.applyChanges());
  EXPECT_EQ(settings.getRosDomainId(), 7);
  EXPECT_FALSE(settings.getHasPendingChanges());
  EXPECT_EQ(applied_changes, 1);

  EXPECT_TRUE(settings.applyChanges());
  EXPECT_EQ(applied_changes, 1);

  std::filesystem::remove_all(test_dir);
}