#include <sstream>
#include <utility>
#include <vector>

#include "Loader/YamlLoader.hpp"

using namespace ROBOGait::loader;

YamlLoader::YamlLoadResult::YamlLoadResult(bool loaded_in) : loaded(loaded_in) {}

YamlLoader::YamlLoadResult::YamlLoadResult(std::string error_in) : loaded(false), error(std::move(error_in)) {}

YamlLoader::YamlLoadResult::operator bool() const { return loaded; }

YamlLoader& YamlLoader::getInstance()
{
  static YamlLoader instance;
  return instance;
}

YamlLoader::YamlLoader() : file_path_(""), is_loaded_(false) {}

YamlLoader::YamlLoadResult YamlLoader::loadConfig(const std::string& file_path)
{
  try
  {
    config_ = YAML::LoadFile(file_path);
    file_path_ = file_path;
    is_loaded_ = true;
    return YamlLoadResult(true);
  }
  catch (const YAML::Exception& e)
  {
    config_ = YAML::Node();
    file_path_.clear();
    is_loaded_ = false;
    return YamlLoadResult(std::string("[YamlLoader::loadConfig] Failed to load configuration from: ") + file_path + ". Error: " + e.what());
  }
}

bool YamlLoader::isLoaded() const { return is_loaded_; }

YAML::Node YamlLoader::getNode(const std::string& key) const
{
  std::vector<std::string> keys;
  std::stringstream ss(key);
  std::string token;

  while (std::getline(ss, token, '.'))
  {
    keys.push_back(token);
  }

  YAML::Node current_node = YAML::Clone(config_);

  for (const auto& k : keys)
  {
    if (!current_node.IsMap())
    {
      return YAML::Node();
    }

    YAML::Node next_node = current_node[k];
    if (!next_node.IsDefined() || next_node.IsNull())
    {
      return YAML::Node();
    }

    current_node = next_node;
  }

  return current_node;
}
