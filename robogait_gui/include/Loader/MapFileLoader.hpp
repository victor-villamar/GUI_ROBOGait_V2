#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <nav_msgs/msg/occupancy_grid.hpp>

namespace ROBOGait
{
namespace loader
{
class MapFileLoader
{
public:
  std::optional<nav_msgs::msg::OccupancyGrid> loadMap(const std::string& yaml_content, const std::vector<uint8_t>& pgm_content) const;

private:
  struct MapMetadata
  {
    double resolution{0.0};
    double origin_x{0.0};
    double origin_y{0.0};
    double origin_theta{0.0};
    int width{0};
    int height{0};
    bool negate{false};
    double occupied_thresh{0.65};
    double free_thresh{0.195};
  };

  static bool readToken(const std::vector<uint8_t>& data, size_t& cursor, std::string_view& token);
  static bool readIntToken(const std::vector<uint8_t>& data, size_t& cursor, int& value);
  bool parseYaml(const std::string& yaml_content, MapMetadata& metadata_out) const;
  bool parsePgm(const std::vector<uint8_t>& pgm_content, int& width_out, int& height_out, std::vector<uint8_t>& pixels_out) const;
  bool buildOccupancyGrid(const MapMetadata& metadata, const std::vector<uint8_t>& pixels, nav_msgs::msg::OccupancyGrid& occupancy_grid_out) const;

  static constexpr int MAX_PIXEL_VALUE = 255;
  static constexpr int ORIGIN_SIZE = 3;
  static constexpr double MAX_THRESHOLD = 1.0;
  static constexpr int8_t OCCUPIED_CELL_VALUE = 100;
  static constexpr int8_t FREE_CELL_VALUE = 0;
  static constexpr int8_t UNKNOWN_CELL_VALUE = -1;
};
} // namespace loader
} // namespace ROBOGait