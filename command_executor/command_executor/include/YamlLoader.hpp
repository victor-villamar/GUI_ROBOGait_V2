#pragma once

#include <string>

#include <yaml-cpp/yaml.h>

namespace ROBOGait
{
namespace loader
{
class YamlLoader
{
public:
  /**
   * @brief Get the singleton instance of the YamlLoader
   *
   * @return YamlLoader& Reference to the singleton instance
   */
  static YamlLoader& getInstance();

  /** Delete copy constructor and assignment operator */
  YamlLoader(const YamlLoader&) = delete;
  YamlLoader& operator=(const YamlLoader&) = delete;

  /**
   * @brief Load the YAML configuration file
   *
   * @param file_path Path to the YAML file
   *
   * @return true if the config was loaded successfully, false otherwise
   */
  bool loadConfig(const std::string& file_path);

  /**
   * @brief Get a value from the YAML configuration
   *
   * @param key The key of the value to retrieve
   * @param default_value The default value to return if the key is not found
   *
   * @return The value associated with the key, or the default value if not found
   */
  template <typename T> T getValue(const std::string& key, const T& default_value = T()) const;

  /**
   * @brief Check if the YAML configuration was loaded successfully
   *
   * @return true if the config was loaded successfully, false otherwise
   */
  bool isLoaded() const;

private:
  /**
   * @brief Constructor for the YamlLoader class
   */
  YamlLoader();

  /**
   * @brief Destructor for the YamlLoader class
   */
  ~YamlLoader() = default;

  /**
   * @brief Get a YAML node from the configuration
   *
   * @param key The key of the node to retrieve
   *
   * @return The YAML node associated with the key, or an empty node if not found
   */
  YAML::Node getNode(const std::string& key) const;

  std::string file_path_; /**< Path to the YAML config file */
  YAML::Node config_;     /**< YAML configuration node */
  bool is_loaded_;        /**< Flag indicating if the config was loaded successfully */
};

template <typename T> T YamlLoader::getValue(const std::string& key, const T& default_value) const
{
  if (!is_loaded_)
  {
    return default_value;
  }

  YAML::Node node = getNode(key);
  if (!node || !node.IsDefined() || node.IsNull())
  {
    return default_value;
  }

  try
  {
    return node.as<T>();
  }
  catch (const YAML::BadConversion&)
  {
    return default_value;
  }
}

} // namespace loader
} // namespace ROBOGait
