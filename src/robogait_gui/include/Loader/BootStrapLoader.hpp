#pragma once

#include <filesystem>
#include <string>

namespace ROBOGait
{
namespace loader
{
/**
 * @brief Class responsible for loading boostrap config from YAML
 */
class BootStrapLoader
{
public:
  /**
   *@brief Struct to hold the boostStrap configuration
   *
   * @param user_config_root_path User configuration root path
   * @param config_file_name config YAML file name
   * @param commands_file_name commands YAML file name
   */
  struct BootStrapConfig
  {
    std::string user_config_root_path;
    std::string config_file_name;
    std::string commands_file_name;
  };

  /**
   * @brief Get the singleton instance of BootStrapLoader
   *
   * @return Reference to the BootStrapLoader instance
   */
  static BootStrapLoader& getInstance();

  /**
   * @brief Delete copy constructor and assignment operator
   */
  BootStrapLoader(const BootStrapLoader&) = delete;
  BootStrapLoader& operator=(const BootStrapLoader&) = delete;

  bool loadBootStrapConfig(const std::filesystem::path& yaml_file);

  /**
   * @brief Get the boostStrap configuration from the YAML file
   *
   * @return Struct containing the configuration parameters
   */
  BootStrapConfig getBootStrapConfig() const;

private:
  BootStrapLoader();
  ~BootStrapLoader() = default;

  BootStrapConfig config_;
};
} // namespace loader
} // namespace ROBOGait