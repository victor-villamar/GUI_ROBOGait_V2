#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Functions.hpp"

namespace ROBOGait
{
namespace functions
{
std::optional<std::string> getMapYamlInfo(const std::string& map_directory, const std::string& map_name)
{
  const std::filesystem::path yaml_file = std::filesystem::path(expandPath(map_directory)) / (map_name + ".yaml");

  if (!std::filesystem::exists(yaml_file))
  {
    std::cerr << "[functions::getMapYamlInfo] YAML file does not exist: " << yaml_file << std::endl;
    return std::nullopt;
  }

  std::ifstream file_stream(yaml_file, std::ios::ate);
  if (!file_stream.is_open())
  {
    std::cerr << "[functions::getMapYamlInfo] Failed to open YAML file: " << yaml_file << std::endl;
    return std::nullopt;
  }

  const std::streamsize size = file_stream.tellg();
  if (size <= 0)
  {
    std::cerr << "[functions::getMapYamlInfo] YAML file is empty or error occurred: " << yaml_file << std::endl;
    return std::nullopt;
  }

  file_stream.seekg(0, std::ios::beg);

  std::string content;
  content.resize(static_cast<size_t>(size));
  if (!file_stream.read(content.data(), size))
  {
    std::cerr << "[functions::getMapYamlInfo] Failed to read YAML file: " << yaml_file << std::endl;
    return std::nullopt;
  }

  return content;
}

std::optional<std::vector<uint8_t>> getMapPgmInfo(const std::string& map_directory, const std::string& map_name)
{
  const std::filesystem::path pgm_file = std::filesystem::path(expandPath(map_directory)) / (map_name + ".pgm");

  if (!std::filesystem::exists(pgm_file))
  {
    std::cerr << "[functions::getMapPgmInfo] PGM file does not exist: " << pgm_file << std::endl;
    return std::nullopt;
  }

  std::ifstream file_stream(pgm_file, std::ios::binary | std::ios::ate);
  if (!file_stream.is_open())
  {
    std::cerr << "[functions::getMapPgmInfo] Failed to open PGM file: " << pgm_file << std::endl;
    return std::nullopt;
  }

  const std::streamsize size = file_stream.tellg();
  if (size <= 0)
  {
    std::cerr << "[functions::getMapPgmInfo] PGM file is empty or error occurred: " << pgm_file << std::endl;
    return std::nullopt;
  }

  file_stream.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(static_cast<size_t>(size));
  if (!file_stream.read(reinterpret_cast<char*>(buffer.data()), size))
  {
    std::cerr << "[functions::getMapPgmInfo] Failed to read PGM file: " << pgm_file << std::endl;
    return std::nullopt;
  }

  return buffer;
}

std::string expandPath(const std::string& path)
{
  if (path.empty())
  {
    return path;
  }

  const char* home_env = std::getenv("HOME");

  if (!home_env)
  {
    return path;
  }

  if (path[0] == '~')
  {
    std::string result(home_env);
    result += path.substr(1);
    return result;
  }

  if (path.compare(0, 5, "$HOME") == 0)
  {
    std::string result(home_env);
    result += path.substr(5);
    return result;
  }

  return path;
}
} // namespace functions
} // namespace ROBOGait