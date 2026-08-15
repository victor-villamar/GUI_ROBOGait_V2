#include <utility>

#include <yaml-cpp/yaml.h>

#include "Loader/BootStrapLoader.hpp"

using namespace ROBOGait::loader;

BootStrapLoader& BootStrapLoader::getInstance()
{
  static BootStrapLoader instance;
  return instance;
}

BootStrapLoader::BootStrapLoader() {}

BootStrapLoader::BootStrapLoadResult::BootStrapLoadResult(BootStrapConfig config_in) : config(std::move(config_in)) {}

BootStrapLoader::BootStrapLoadResult::BootStrapLoadResult(std::string error_in) : error(std::move(error_in)) {}

BootStrapLoader::BootStrapLoadResult::operator bool() const { return config.has_value(); }

BootStrapLoader::BootStrapLoadResult BootStrapLoader::loadBootStrapConfig(const std::filesystem::path& yaml_file)
{
  if (!std::filesystem::exists(yaml_file))
  {
    return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Bootstrap YAML does not exist: " + yaml_file.string());
  }

  BootStrapConfig parsed_config;

  try
  {
    YAML::Node bootstrap_config = YAML::LoadFile(yaml_file.string());

    if (!bootstrap_config)
    {
      return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Failed to load bootstrap configuration from: " + yaml_file.string());
    }

    // Get config file path
    if (!bootstrap_config["user_config_root"])
    {
      return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] User configuration root path not configured in YAML");
    }

    if (!bootstrap_config["config_file"])
    {
      return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Config file name not configured in YAML");
    }

    if (!bootstrap_config["commands_file"])
    {
      return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Commands file name not configured in YAML");
    }

    // Get values from YAML
    parsed_config.user_config_root_path = bootstrap_config["user_config_root"].as<std::string>();
    parsed_config.config_file_name = bootstrap_config["config_file"].as<std::string>();
    parsed_config.commands_file_name = bootstrap_config["commands_file"].as<std::string>();
  }
  catch (const YAML::Exception& exception)
  {
    return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Failed to parse bootstrap YAML: " + yaml_file.string() + " " + exception.what());
  }

  if (parsed_config.user_config_root_path.empty())
  {
    return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] User configuration root path is not defined");
  }

  if (parsed_config.config_file_name.empty())
  {
    return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Config file name is not defined");
  }
  if (parsed_config.commands_file_name.empty())
  {
    return BootStrapLoadResult("[BootStrapLoader::loadBootStrapConfig] Commands file name is not defined");
  }

  config_ = parsed_config;
  return BootStrapLoadResult(std::move(parsed_config));
}

BootStrapLoader::BootStrapConfig BootStrapLoader::getBootStrapConfig() const { return config_; }