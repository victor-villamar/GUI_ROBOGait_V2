#include <iostream>

#include <yaml-cpp/yaml.h>

#include "Loader/BootStrapLoader.hpp"

using namespace ROBOGait::loader;

BootStrapLoader& BootStrapLoader::getInstance()
{
  static BootStrapLoader instance;
  return instance;
}

BootStrapLoader::BootStrapLoader() {}

bool BootStrapLoader::loadBootStrapConfig(const std::filesystem::path& yaml_file)
{
  if (!std::filesystem::exists(yaml_file))
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] Bootstrap YAML does not exist:" << yaml_file << std::endl;
    return false;
  }

  YAML::Node boostrap_config = YAML::LoadFile(yaml_file.string());

  if (!boostrap_config)
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] Failed to load bootstrap configuration from:" << yaml_file << std::endl;
    return {};
  }

  // Get config file path
  if (!boostrap_config["user_config_root"])
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] User configuration root path not configured in YAML";
    return false;
  }

  if (!boostrap_config["config_file"])
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] Config file name not configured in YAML";
    return false;
  }

  if (!boostrap_config["commands_file"])
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] Commands file name not configured in YAML" << std::endl;
    return false;
  }

  // Get values from YAML
  config_.user_config_root_path = boostrap_config["user_config_root"].as<std::string>();
  config_.config_file_name = boostrap_config["config_file"].as<std::string>();
  config_.commands_file_name = boostrap_config["commands_file"].as<std::string>();

  if (config_.user_config_root_path.empty())
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] User configuration root path is not defined" << std::endl;
    return false;
  }

  if (config_.config_file_name.empty())
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] Config file name is not defined" << std::endl;
    return false;
  }
  if (config_.commands_file_name.empty())
  {
    std::cout << "[BootStrapLoader::loadBootStrapConfig] Commands file name is not defined" << std::endl;
    return false;
  }

  return true;
}

BootStrapLoader::BootStrapConfig BootStrapLoader::getBootStrapConfig() const { return config_; }