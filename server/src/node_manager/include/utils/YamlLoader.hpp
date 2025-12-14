#pragma once

#include <optional>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "../common/structs.hpp"

namespace ROBOGait
{
namespace loader
{

class YamlLoader
{
public:
  YamlLoader();
  ~YamlLoader();
  bool loadFromFile(const std::string& path);
  std::optional<common::ProcessConfig> getProcess(const std::string& key, bool required = true) const;
  common::DatabaseConfig getDatabaseConfig() const;
  std::string getPathToMaps() const;
  std::string getType() const;

private:
  YAML::Node config_;
  std::string root_path_;
};

} // namespace loader
} // namespace ROBOGait
