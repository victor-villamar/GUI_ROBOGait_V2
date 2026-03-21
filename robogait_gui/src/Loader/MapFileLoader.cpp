#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>

#include <tf2/LinearMath/Quaternion.h>
#include <yaml-cpp/yaml.h>

#include "Loader/MapFileLoader.hpp"

using namespace ROBOGait::loader;

std::optional<nav_msgs::msg::OccupancyGrid> MapFileLoader::loadMap(const std::string& yaml_content, const std::vector<uint8_t>& pgm_content) const
{
  MapMetadata metadata;

  if (!parseYaml(yaml_content, metadata))
  {
    std::cerr << "[MapFileLoader::loadMap] Failed to parse YAML content" << std::endl;
    return std::nullopt;
  }

  std::vector<uint8_t> pixels;
  if (!parsePgm(pgm_content, metadata.width, metadata.height, pixels))
  {
    std::cerr << "[MapFileLoader::loadMap] Failed to parse PGM content" << std::endl;
    return std::nullopt;
  }

  nav_msgs::msg::OccupancyGrid occupancy_grid;
  if (!buildOccupancyGrid(metadata, pixels, occupancy_grid))
  {
    std::cerr << "[MapFileLoader::loadMap] Failed to build OccupancyGrid message" << std::endl;
    return std::nullopt;
  }

  return occupancy_grid;
}

bool MapFileLoader::readToken(const std::vector<uint8_t>& data, size_t& cursor, std::string_view& token)
{
  token = std::string_view{};
  const size_t n = data.size();

  while (cursor < n)
  {
    // clang-format off
    while (cursor < n &&
          (data[cursor] == ' ' || data[cursor] == '\t' ||
           data[cursor] == '\n' || data[cursor] == '\r'))
    {
      ++cursor;
    }
    // clang-format on

    if (cursor < n && data[cursor] == '#')
    {
      while (cursor < n && data[cursor] != '\n')
      {
        ++cursor;
      }
      continue;
    }

    break;
  }

  if (cursor >= n)
  {
    return false;
  }

  const size_t start = cursor;

  // clang-format off
  while (cursor < n &&
         data[cursor] != ' ' && data[cursor] != '\t' &&
         data[cursor] != '\n' && data[cursor] != '\r' &&
         data[cursor] != '#')
  {
    ++cursor;
  }
  // clang-format on

  token = std::string_view(reinterpret_cast<const char*>(data.data() + start), cursor - start);
  return !token.empty();
}

bool MapFileLoader::readIntToken(const std::vector<uint8_t>& data, size_t& cursor, int& value)
{
  std::string_view tok;

  if (!readToken(data, cursor, tok))
    return false;

  const char* begin = tok.data();
  const char* end = tok.data() + tok.size();

  const auto result = std::from_chars(begin, end, value);

  return result.ec == std::errc{} && result.ptr == end;
}

bool MapFileLoader::parseYaml(const std::string& yaml_content, MapMetadata& metadata_out) const
{
  try
  {
    const YAML::Node node = YAML::Load(yaml_content);
    if (!node || !node.IsMap())
    {
      std::cerr << "[MapFileLoader::parseYaml] Invalid YAML root node" << std::endl;
      return false;
    }

    const YAML::Node resolution_node = node["resolution"];
    const YAML::Node origin_node = node["origin"];
    const YAML::Node mode_node = node["mode"];

    if (!resolution_node)
    {
      std::cerr << "[MapFileLoader::parseYaml] Missing 'resolution' field in YAML" << std::endl;
      return false;
    }

    if (!origin_node || !origin_node.IsSequence() || origin_node.size() != ORIGIN_SIZE)
    {
      std::cerr << "[MapFileLoader::parseYaml] Missing or invalid 'origin' field in YAML" << std::endl;
      return false;
    }

    metadata_out.resolution = resolution_node.as<double>();

    if (metadata_out.resolution <= 0.0)
    {
      std::cerr << "[MapFileLoader::parseYaml] Resolution must be a positive value" << std::endl;
      return false;
    }

    metadata_out.origin_x = origin_node[0].as<double>();
    metadata_out.origin_y = origin_node[1].as<double>();
    metadata_out.origin_theta = origin_node[2].as<double>();

    if (const YAML::Node negate_node = node["negate"])
    {
      metadata_out.negate = negate_node.as<int>() != 0;
    }

    if (mode_node && mode_node.IsScalar())
    {
      std::string mode_value = mode_node.as<std::string>();
      std::transform(mode_value.begin(), mode_value.end(), mode_value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

      if (mode_value == "trinary")
      {
        metadata_out.mode = MapMetadata::MapMode::Trinary;
      }
      else if (mode_value == "raw")
      {
        metadata_out.mode = MapMetadata::MapMode::Raw;
      }
      else
      {
        metadata_out.mode = MapMetadata::MapMode::Scale;
      }
    }

    if (const YAML::Node occupied_thresh_node = node["occupied_thresh"])
    {
      metadata_out.occupied_thresh = occupied_thresh_node.as<double>();
    }

    if (const YAML::Node free_thresh_node = node["free_thresh"])
    {
      metadata_out.free_thresh = free_thresh_node.as<double>();
    }

    if (metadata_out.occupied_thresh < 0.0 || metadata_out.occupied_thresh > MAX_THRESHOLD)
    {
      std::cerr << "[MapFileLoader::parseYaml] 'occupied_thresh' must be in range [0.0, " << MAX_THRESHOLD << "]" << std::endl;
      return false;
    }

    if (metadata_out.free_thresh < 0.0 || metadata_out.free_thresh > MAX_THRESHOLD)
    {
      std::cerr << "[MapFileLoader::parseYaml] 'free_thresh' must be in range [0.0, " << MAX_THRESHOLD << "]" << std::endl;
      return false;
    }

    if (metadata_out.free_thresh >= metadata_out.occupied_thresh)
    {
      std::cerr << "[MapFileLoader::parseYaml] 'free_thresh' must be less than 'occupied_thresh'" << std::endl;
      return false;
    }

    return true;
  }
  catch (const std::exception& e)
  {
    std::cerr << "[MapFileLoader::parseYaml] Exception while parsing YAML: " << e.what() << std::endl;
    return false;
  }
}

bool MapFileLoader::parsePgm(const std::vector<uint8_t>& pgm_content, int& width_out, int& height_out, std::vector<uint8_t>& pixels_out) const
{
  size_t cursor = 0;
  std::string_view token;

  if (!readToken(pgm_content, cursor, token) || token != "P5")
  {
    std::cerr << "[MapFileLoader::parsePgm] Invalid PGM header (expected 'P5')" << std::endl;
    return false;
  }

  if (!readIntToken(pgm_content, cursor, width_out) || width_out <= 0)
  {
    std::cerr << "[MapFileLoader::parsePgm] Failed to read valid width from PGM header" << std::endl;
    return false;
  }

  if (!readIntToken(pgm_content, cursor, height_out) || height_out <= 0)
  {
    std::cerr << "[MapFileLoader::parsePgm] Failed to read valid height from PGM header" << std::endl;
    return false;
  }

  int max_value = 0;

  if (!readIntToken(pgm_content, cursor, max_value) || max_value <= 0 || max_value > MAX_PIXEL_VALUE)
  {
    std::cerr << "[MapFileLoader::parsePgm] Failed to read valid max pixel value from PGM header" << std::endl;
    return false;
  }

  if (static_cast<size_t>(width_out) > (std::numeric_limits<size_t>::max() / static_cast<size_t>(height_out)))
  {
    std::cerr << "[MapFileLoader::parsePgm] Image dimensions are too large and cause size overflow" << std::endl;
    return false;
  }

  if (cursor >= pgm_content.size() || !std::isspace(static_cast<unsigned char>(pgm_content[cursor])))
  {
    std::cerr << "[MapFileLoader::parsePgm] Expected whitespace after max pixel value in PGM header" << std::endl;
    return false;
  }

  if (pgm_content[cursor] == '\r')
  {
    cursor++;
    if (cursor < pgm_content.size() && pgm_content[cursor] == '\n')
    {
      ++cursor;
    }
  }
  else
  {
    ++cursor;
  }

  const size_t expected_size = static_cast<size_t>(width_out) * static_cast<size_t>(height_out);
  if (cursor + expected_size > pgm_content.size())
  {
    std::cerr << "[MapFileLoader::parsePgm] PGM pixel data is incomplete based on expected image dimensions" << std::endl;
    return false;
  }

  pixels_out.resize(expected_size);
  std::memcpy(pixels_out.data(), pgm_content.data() + cursor, expected_size);

  if (max_value != MAX_PIXEL_VALUE)
  {
    const double scale = static_cast<double>(MAX_PIXEL_VALUE) / static_cast<double>(max_value);
    for (auto& pixel_out : pixels_out)
    {
      pixel_out = static_cast<uint8_t>(std::round(static_cast<double>(pixel_out) * scale));
    }
  }

  return true;
}

bool MapFileLoader::buildOccupancyGrid(const MapMetadata& metadata, const std::vector<uint8_t>& pixels, nav_msgs::msg::OccupancyGrid& occupancy_grid_out) const
{
  if (metadata.width <= 0 || metadata.height <= 0)
  {
    std::cerr << "[MapFileLoader::buildOccupancyGrid] Invalid map dimensions" << std::endl;
    return false;
  }

  const size_t width = static_cast<size_t>(metadata.width);
  const size_t height = static_cast<size_t>(metadata.height);
  const size_t expected_size = width * height;

  if (pixels.size() != expected_size)
  {
    std::cerr << "[MapFileLoader::buildOccupancyGrid] Pixel data size does not match map dimensions" << std::endl;
    return false;
  }

  occupancy_grid_out.info.width = static_cast<uint32_t>(metadata.width);
  occupancy_grid_out.info.height = static_cast<uint32_t>(metadata.height);
  occupancy_grid_out.info.resolution = static_cast<float>(metadata.resolution);

  occupancy_grid_out.info.origin.position.x = metadata.origin_x;
  occupancy_grid_out.info.origin.position.y = metadata.origin_y;
  occupancy_grid_out.info.origin.position.z = 0.0;

  tf2::Quaternion quaternion;
  quaternion.setRPY(0.0, 0.0, metadata.origin_theta);

  occupancy_grid_out.info.origin.orientation.x = quaternion.x();
  occupancy_grid_out.info.origin.orientation.y = quaternion.y();
  occupancy_grid_out.info.origin.orientation.z = quaternion.z();
  occupancy_grid_out.info.origin.orientation.w = quaternion.w();

  occupancy_grid_out.data.assign(expected_size, UNKNOWN_CELL_VALUE);

  const double inv_max_pixel_value = 1.0 / static_cast<double>(MAX_PIXEL_VALUE);
  const double occupied_threshold = metadata.occupied_thresh;
  const double free_threshold = metadata.free_thresh;
  const bool negate = metadata.negate;

  if (metadata.mode == MapMetadata::MapMode::Trinary)
  {
    for (size_t y = 0; y < height; ++y)
    {
      const size_t src_y = height - 1 - y;
      const size_t src_row = src_y * width;
      const size_t dst_row = y * width;

      for (size_t x = 0; x < width; ++x)
      {
        const uint8_t pixel_value = pixels[src_row + x];
        int8_t cell = UNKNOWN_CELL_VALUE;

        if (!negate)
        {
          if (pixel_value == 0)
          {
            cell = OCCUPIED_CELL_VALUE;
          }
          else if (pixel_value >= PARTIALLY_OCCUPIED_CELL_VALUE)
          {
            cell = FREE_CELL_VALUE;
          }
        }
        else
        {
          if (pixel_value == 0)
          {
            cell = FREE_CELL_VALUE;
          }
          else if (pixel_value >= PARTIALLY_OCCUPIED_CELL_VALUE)
          {
            cell = OCCUPIED_CELL_VALUE;
          }
        }

        occupancy_grid_out.data[dst_row + x] = cell;
      }
    }

    return true;
  }

  if (metadata.mode == MapMetadata::MapMode::Raw)
  {
    for (size_t y = 0; y < height; ++y)
    {
      const size_t src_y = height - 1 - y;
      const size_t src_row = src_y * width;
      const size_t dst_row = y * width;

      for (size_t x = 0; x < width; ++x)
      {
        const uint8_t pixel_value = pixels[src_row + x];
        int8_t cell = UNKNOWN_CELL_VALUE;

        if (pixel_value != MAX_PIXEL_VALUE)
        {
          int value = static_cast<int>(pixel_value);
          if (value > OCCUPIED_CELL_VALUE)
          {
            value = OCCUPIED_CELL_VALUE;
          }
          if (negate)
          {
            value = OCCUPIED_CELL_VALUE - value;
          }
          cell = static_cast<int8_t>(value);
        }

        occupancy_grid_out.data[dst_row + x] = cell;
      }
    }

    return true;
  }

  for (size_t y = 0; y < height; ++y)
  {
    const size_t src_y = height - 1 - y;
    const size_t src_row = src_y * width;
    const size_t dst_row = y * width;

    for (size_t x = 0; x < width; ++x)
    {
      const uint8_t pixel_value = pixels[src_row + x];

      double occupancy = 1.0 - static_cast<double>(pixel_value) * inv_max_pixel_value;
      if (negate)
      {
        occupancy = 1.0 - occupancy;
      }

      int8_t cell = UNKNOWN_CELL_VALUE;

      if (occupancy > occupied_threshold)
      {
        cell = OCCUPIED_CELL_VALUE;
      }
      else if (occupancy < free_threshold)
      {
        cell = FREE_CELL_VALUE;
      }

      occupancy_grid_out.data[dst_row + x] = cell;
    }
  }

  return true;
}
