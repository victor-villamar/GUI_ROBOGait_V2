#pragma once

#include <iostream>
#include <string>

#include <yaml-cpp/yaml.h>

namespace ROBOGait
{
namespace loader
{

/**
 * @brief Singleton class for loading and accessing YAML configuration
 *
 * This class provides centralized access to configuration parameters
 * stored in a YAML file.
 */
class YamlLoader
{
public:
  /**
   * @brief Get the singleton instance
   *
   * @return Reference to the YamlLoader instance
   */
  static YamlLoader& getInstance();

  /**
   * @brief Delete copy constructor and assignment operator
   */
  YamlLoader(const YamlLoader&) = delete;
  YamlLoader& operator=(const YamlLoader&) = delete;

  /**
   * @brief Load configuration from YAML file
   *
   * @param file_path Absolute path to the YAML configuration file
   *
   * @return true if loading was successful, false otherwise
   */
  bool loadConfig(const std::string& file_path);

  /**
   * @brief Get a value from the configuration
   *
   * @tparam T Type of the value to retrieve
   * @param key Configuration key (supports nested keys with dot notation, e.g., "database.path")
   * @param default_value Default value if key is not found
   *
   * @return Configuration value or default value
   */
  template <typename T> T getValue(const std::string& key, const T& default_value = T()) const;

  /**
   * @brief Check if configuration has been loaded
   *
   * @return true if configuration is loaded, false otherwise
   */
  bool isLoaded() const;

private:
  YamlLoader();
  ~YamlLoader() = default;

  /**
   * @brief Get YAML node by key path
   *
   * @param key Configuration key path
   *
   * @return YAML node if found, undefined node otherwise
   */
  YAML::Node getNode(const std::string& key) const;

  std::string file_path_; /**< Path to the YAML configuration file */
  YAML::Node config_;     /**< Parsed YAML configuration */
  bool is_loaded_;        /**< Flag indicating if configuration is loaded */
};

// Template implementation
template <typename T> T YamlLoader::getValue(const std::string& key, const T& default_value) const
{
  if (!is_loaded_)
  {
    std::cerr << "[YamlLoader::getValue] Configuration not loaded, returning default value for key: " << key << std::endl;
    return default_value;
  }

  YAML::Node node = getNode(key);
  if (node && node.IsDefined())
  {
    return node.as<T>();
  }

  return default_value;
}

} // namespace loader
} // namespace ROBOGait