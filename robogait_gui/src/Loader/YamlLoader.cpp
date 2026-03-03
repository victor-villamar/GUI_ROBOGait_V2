#include <QDebug>
#include <sstream>
#include <vector>

#include "Loader/YamlLoader.hpp"

using namespace ROBOGait::loader;

YamlLoader& YamlLoader::getInstance()
{
  static YamlLoader instance;
  return instance;
}

YamlLoader::YamlLoader() : file_path_(""), is_loaded_(false) { qInfo() << "[YamlLoader::YamlLoader] YamlLoader instance created"; }

bool YamlLoader::loadConfig(const std::string& file_path)
{
  config_ = YAML::LoadFile(file_path);
  file_path_ = file_path;
  is_loaded_ = true;
  qInfo() << "[YamlLoader::loadConfig] Configuration loaded successfully from:" << QString::fromStdString(file_path);
  return true;
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
    if (!current_node[k])
    {
      return YAML::Node();
    }
    current_node = current_node[k];
  }

  return current_node;
}
