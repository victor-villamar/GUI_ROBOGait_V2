#pragma once

#include <optional>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "common/structs.hpp"

namespace ROBOGait
{
namespace loader
{

class YamlLoader
{
public:
  /**
   * @brief Construct a new YamlLoader object
   */
  YamlLoader();

  /**
   * @brief Destroy the YamlLoader object
   */
  ~YamlLoader();

  /**
   * @brief Load YAML configuration from a file
   *
   * @param path The path to the YAML file
   * @return true if loading was successful, false otherwise
   */
  bool loadFromFile(const std::string& path);

  /**
   * @brief Get the Process configuration for a given key
   *
   * @param key The key to look for
   * @return The Process configuration if found, std::nullopt otherwise
   */
  std::optional<common::ProcessConfig> getProcess(const std::string& key) const;

  /**
   * @brief Get the Database configuration
   *
   * @return The Database configuration
   */
  common::DatabaseConfig getDatabaseConfig() const;

  /**
   * @brief Get the path to the maps directory
   *
   * @return The path to the maps directory
   */
  std::string getPathToMaps() const;

  /**
   * @brief Get the type of the configuration
   *
   * @return The type of the configuration
   */
  std::string getType() const;

private:
  YAML::Node config_;     /**< YAML configuration node */
  std::string root_path_; /**< Root path for the configuration */
};

} // namespace loader
} // namespace ROBOGait
