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
  static YamlLoader& getInstance();

  YamlLoader(const YamlLoader&) = delete;
  YamlLoader& operator=(const YamlLoader&) = delete;

  bool loadConfig(const std::string& file_path);

  template <typename T> T getValue(const std::string& key, const T& default_value = T()) const;

  bool isLoaded() const;

private:
  YamlLoader();
  ~YamlLoader() = default;

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
  if (node && node.IsDefined())
  {
    try
    {
      return node.as<T>();
    }
    catch (const YAML::BadConversion&)
    {
      return default_value;
    }
  }

  return default_value;
}

} // namespace loader
} // namespace ROBOGait
