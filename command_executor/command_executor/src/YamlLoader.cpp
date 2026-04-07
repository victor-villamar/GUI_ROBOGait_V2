#include <sstream>
#include <vector>

#include "YamlLoader.hpp"

using namespace ROBOGait::loader;

YamlLoader& YamlLoader::getInstance()
{
  static YamlLoader instance;
  return instance;
}

YamlLoader::YamlLoader() : file_path_(""), is_loaded_(false) {}

bool YamlLoader::loadConfig(const std::string& file_path)
{
  try
  {
    config_ = YAML::LoadFile(file_path);
    file_path_ = file_path;
    is_loaded_ = true;
    return true;
  }
  catch (const std::exception&)
  {
    config_ = YAML::Node();
    file_path_.clear();
    is_loaded_ = false;
    return false;
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

  std::vector<YAML::Node> chain;
  chain.reserve(keys.size() + 1);
  chain.push_back(config_);

  for (const auto& k : keys)
  {
    const YAML::Node& current = chain.back();
    if (!current.IsDefined())
    {
      return YAML::Node();
    }
    if (!current.IsMap())
    {
      return YAML::Node();
    }

    const YAML::Node& current_const = current;
    YAML::Node next_node = current_const[k];
    if (!next_node.IsDefined())
    {
      return YAML::Node();
    }

    chain.push_back(next_node);
  }

  return chain.back();
}
