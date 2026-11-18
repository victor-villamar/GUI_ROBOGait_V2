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
/**
 * @brief Class responsible for loading map data from YAML and PGM content and converting it into a ROS OccupancyGrid message
 */
class MapFileLoader
{
public:
  /**
   * @brief Result returned when loading a map from YAML and PGM content
   */
  struct MapLoadResult
  {
    explicit MapLoadResult(nav_msgs::msg::OccupancyGrid occupancy_grid_in);
    explicit MapLoadResult(std::string error_in);

    std::optional<nav_msgs::msg::OccupancyGrid> occupancy_grid;
    std::string error;

    explicit operator bool() const;
  };

  /**
   * @brief Load map data from YAML content and PGM content, and convert it into a ROS OccupancyGrid message
   *
   * @param yaml_content The content of the YAML file as a string
   * @param pgm_content The content of the PGM file as a vector of bytes
   *
   * @return The loaded OccupancyGrid message if successful, or an error message if there was an error during loading or parsing
   */
  MapLoadResult loadMap(const std::string& yaml_content, const std::vector<uint8_t>& pgm_content) const;

private:
  /**
   * @brief Struct to hold metadata extracted from the YAML file
   *
   * @param resolution The resolution of the map in meters per pixel
   * @param origin_x The x-coordinate of the map origin in the world frame
   * @param origin_y The y-coordinate of the map origin in the world frame
   * @param origin_theta The orientation of the map origin in radians
   * @param width The width of the map in pixels
   * @param height The height of the map in pixels
   * @param negate Whether the pixel values should be negated (i.e., 0 becomes 255 and 255 becomes 0)
   * @param occupied_thresh The threshold above which a cell is considered occupied (as a fraction of the maximum pixel value)
   * @param free_thresh The threshold below which a cell is considered free (as a fraction of the maximum pixel value)
   * @param mode The mode for interpreting pixel values (Scale, Trinary, or Raw)
   */
  struct MapMetadata
  {
    /**
     * @brief Enum to specify how pixel values should be interpreted when converting to occupancy values
     *
     */
    enum class MapMode
    {
      Scale,   /**< Scale mode */
      Trinary, /**< Trinary mode */
      Raw      /**< Raw mode */
    };

    double resolution{0.0};
    double origin_x{0.0};
    double origin_y{0.0};
    double origin_theta{0.0};
    int width{0};
    int height{0};
    bool negate{false};
    double occupied_thresh{0.65};
    double free_thresh{0.195};
    MapMode mode{MapMode::Scale};
  };

  /**
   * @brief Read a token from the given data starting at the specified cursor position, and update the cursor to the position after the token
   *
   * @param data The input data as a vector of bytes
   * @param cursor The current position in the data, which will be updated to the position after the token is read
   * @param token_out The output string_view that will point to the read token
   *
   * @return true if a token was successfully read, false if there was an error
   */
  static bool readToken(const std::vector<uint8_t>& data, size_t& cursor, std::string_view& token);

  /**
   * @brief Read an integer token from the given data starting at the specified cursor position, and update the cursor to the position after the token
   *
   * @param data The input data as a vector of bytes
   * @param cursor The current position in the data, which will be updated to the position after the token is read
   * @param value The output integer that will hold the parsed value
   *
   * @return true if an integer token was successfully read and parsed, false if there was an error
   */
  static bool readIntToken(const std::vector<uint8_t>& data, size_t& cursor, int& value);

  /**
   * @brief Parse the YAML content to extract map metadata
   *
   * @param yaml_content The content of the YAML file as a string
   * @param metadata_out The output MapMetadata struct that will hold the extracted metadata
   *
   * @return true if the YAML content was successfully parsed and metadata was extracted, false if there was an error
   */
  bool parseYaml(const std::string& yaml_content, MapMetadata& metadata_out, std::string& error_out) const;

  /**
   * @brief Parse the PGM content to extract pixel data and dimensions
   *
   * @param pgm_content The content of the PGM file as a vector of bytes
   * @param width_out The output width of the map in pixels
   * @param height_out The output height of the map in pixels
   * @param pixels_out The output vector of bytes that will hold the pixel data
   *
   * @return true if the PGM content was successfully parsed and pixel data was extracted, false if there was an error
   */
  bool parsePgm(const std::vector<uint8_t>& pgm_content, int& width_out, int& height_out, std::vector<uint8_t>& pixels_out, std::string& error_out) const;

  /**
   * @brief Build an OccupancyGrid message from the given map metadata and pixel data
   *
   * @param metadata The map metadata extracted from the YAML file
   * @param pixels The pixel data extracted from the PGM file
   * @param occupancy_grid_out The output OccupancyGrid message that will be populated with the converted occupancy data
   *
   * @return true if the OccupancyGrid message was successfully built, false if there was an error during conversion
   */
  bool buildOccupancyGrid(const MapMetadata& metadata, const std::vector<uint8_t>& pixels, nav_msgs::msg::OccupancyGrid& occupancy_grid_out,
                          std::string& error_out) const;

  static constexpr int MAX_PIXEL_VALUE = 255;        /**< Maximum value for a pixel in the map */
  static constexpr int ORIGIN_SIZE = 3;              /**< Expected number of values in the origin field of the YAML file (x, y, theta) */
  static constexpr double MAX_THRESHOLD = 1.0;       /**< Maximum threshold value for occupied and free thresholds (as a fraction of the maximum pixel value) */
  static constexpr int8_t OCCUPIED_CELL_VALUE = 100; /**< Occupancy value for cells considered occupied */
  static constexpr int8_t FREE_CELL_VALUE = 0;       /**< Occupancy value for cells considered free */
  static constexpr int8_t UNKNOWN_CELL_VALUE = -1;   /**< Occupancy value for cells considered unknown */
  static constexpr uint8_t PARTIALLY_OCCUPIED_CELL_VALUE =
      254; /**< Occupancy value for cells that are partially occupied (used in Scale mode when pixel values are between free and occupied thresholds) */
};
} // namespace loader
} // namespace ROBOGait