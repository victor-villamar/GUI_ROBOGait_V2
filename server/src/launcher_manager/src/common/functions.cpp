#include <ament_index_cpp/get_package_share_directory.hpp>
#include <filesystem>
#include <logs/Logs.hpp>

#include "common/functions.hpp"

using namespace ROBOGait::common::logs;

namespace ROBOGait
{
namespace common
{
namespace functions
{
std::string getFilePath(const std::string& package_name, const std::string& relative_path)
{
  const std::string share_path = ament_index_cpp::get_package_share_directory(package_name);
  const std::string full_path = share_path + "/" + relative_path;

  if (!std::filesystem::exists(full_path))
  {
    Logs::error("[functions::getFilePath] File does not exist: %s", full_path.c_str());
    return "";
  }

  return full_path;
}

std::string replaceSpaces(const std::string& name)
{
  std::string modified_name = name;
  for (auto& character : modified_name)
  {
    if (character == ' ')
    {
      character = '#'; // Use a special character, such as '#'
    }
  }
  return modified_name;
}

void deleteMapFile(const std::string& map_path, const std::string& map_name)
{
  const std::string encoded_name = replaceSpaces(map_name);

  std::filesystem::path yaml = std::filesystem::path(map_path) / (encoded_name + ".yaml");
  std::filesystem::path pgm = std::filesystem::path(map_path) / (encoded_name + ".pgm");

  std::error_code ec;
  bool yaml_removed = std::filesystem::remove(yaml, ec);
  if (!ec)
  {
    if (yaml_removed)
    {
      Logs::info("[functions::deleteMapFile] Deleted map file: %s", yaml.string().c_str());
    }
    else
    {
      Logs::error("[functions::deleteMapFile] Map file not found for deletion: %s", yaml.string().c_str());
    }
  }
  else
  {
    Logs::error("[functions::deleteMapFile] Error deleting map file: %s, error: %s", yaml.string().c_str(), ec.message().c_str());
  }

  bool pgm_removed = std::filesystem::remove(pgm, ec);
  if (!ec)
  {
    if (pgm_removed)
    {
      Logs::info("[functions::deleteMapFile] Deleted map file: %s", pgm.string().c_str());
    }
    else
    {
      Logs::error("[functions::deleteMapFile] Map file not found for deletion: %s", pgm.string().c_str());
    }
  }
  else
  {
    Logs::error("[functions::deleteMapFile] Error deleting map file: %s, error: %s", pgm.string().c_str(), ec.message().c_str());
  }
}

} // namespace functions
} // namespace common
} // namespace ROBOGait