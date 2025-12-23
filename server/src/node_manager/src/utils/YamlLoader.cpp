#include <algorithm>
#include <cctype>
#include <iostream>
#include <logs/Logs.hpp>

#include "utils/YamlLoader.hpp"

using namespace ROBOGait::common;
using namespace ROBOGait::loader;
using namespace ROBOGait::common::logs;

namespace
{
/**
 * @brief Parse the window state from a YAML node
 * @param node The YAML node containing the window state
 * @return The parsed WindowState enum value
 */
WindowState parseWindowState(const YAML::Node& node)
{

  if (!node)
  {
    Logs::infoStream("[YamlLoader::parseWindowState] Window state is null, defaulting to MAXIMIZED");
    return WindowState::MAXIMIZED;
  }

  if (!node.IsScalar())
  {
    Logs::infoStream("[YamlLoader::parseWindowState] Window state is not a scalar, defaulting to MAXIMIZED");
    return WindowState::MAXIMIZED;
  }

  std::string value = node.as<std::string>();

  for (auto& c : value)
  {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }

  if (value == "minimized")
  {
    return WindowState::MINIMIZED;
  }
  if (value == "maximized")
  {
    return WindowState::MAXIMIZED;
  }

  return WindowState::MAXIMIZED;
}
} // namespace

YamlLoader::YamlLoader()
{
  root_path_ = "";
  config_ = YAML::Node();
}

YamlLoader::~YamlLoader() {}

bool YamlLoader::loadFromFile(const std::string& path)
{
  root_path_ = path + "../";
  try
  {
    config_ = YAML::LoadFile(path + "config.yaml");
  }
  catch (const std::exception& e)
  {
    Logs::error("[YamlLoader::loadFromFile] Error loading YAML file: %s", e.what());
    return false;
  }
  return true;
}

std::optional<ProcessConfig> YamlLoader::getProcess(const std::string& key) const
{
  if (!config_ || !config_["PROCESSES"] || !config_["PROCESSES"][key])
  {
    Logs::errorStream("[YamlLoader::getProcess] Process configuration for '" + key + "' not found in config.yaml");
    return std::nullopt;
  }

  const auto node = config_["PROCESSES"][key];
  common::ProcessConfig definition;
  definition.name = node["name"].as<std::string>();
  definition.executable = node["executable"].as<std::string>();
  if (node["arguments"])
  {
    definition.arguments = node["arguments"].as<std::vector<std::string>>();
  }
  definition.window_state = ::parseWindowState(node["window_state"]);
  return definition;
}

DatabaseConfig YamlLoader::getDatabaseConfig() const
{
  DatabaseConfig config;
  config.name = config_["NAME_DATABASE"].as<std::string>();
  config.path = root_path_ + config_["DATABASE"].as<std::string>();
  config.script = root_path_ + config_["PATH2DATABASE"].as<std::string>();
  return config;
}

std::string YamlLoader::getPathToMaps() const { return root_path_ + config_["PATH2MAP"].as<std::string>(); }

std::string YamlLoader::getType() const
{
  if (!config_["TYPE"])
  {
    return {};
  }
  return config_["TYPE"].as<std::string>();
}
