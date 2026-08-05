#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <utility>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <Qt>
#include <QtGlobal>

#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

#include "Loader/YamlLoader.hpp"
#include "Map/Utils/Utils.hpp"
#include "Themes/AppTheme.hpp"

namespace ROBOGait
{
namespace map
{
namespace utils
{
namespace
{
double distanceSquared(const WaypointInput& first, const WaypointInput& second)
{
  const double dx = first.x - second.x;
  const double dy = first.y - second.y;
  return dx * dx + dy * dy;
}

std::optional<double> calculateWaypointYaw(const std::vector<WaypointInput>& points, size_t index)
{
  if (points.size() < 2)
  {
    return std::nullopt;
  }

  const size_t last_index = points.size() - 1U;
  const size_t clamped_index = std::min(index, last_index);
  const size_t previous_index = clamped_index > 0U ? clamped_index - 1U : clamped_index;
  const size_t next_index = clamped_index < last_index ? clamped_index + 1U : clamped_index;

  WaypointInput direction_start;
  WaypointInput direction_end;
  if (clamped_index > 0U)
  {
    direction_start = points[previous_index];
    direction_end = points[clamped_index];
  }
  else
  {
    direction_start = points[clamped_index];
    direction_end = points[next_index];
  }

  const double dx = direction_end.x - direction_start.x;
  const double dy = direction_end.y - direction_start.y;
  if (std::abs(dx) <= 1e-9 && std::abs(dy) <= 1e-9)
  {
    return std::nullopt;
  }

  return std::atan2(dy, dx);
}
} // namespace

double getYaw(const geometry_msgs::msg::Quaternion& quaternion)
{
  tf2::Quaternion tf_quat(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

  double roll, pitch, yaw;
  tf2::Matrix3x3(tf_quat).getRPY(roll, pitch, yaw);

  return yaw;
}

geometry_msgs::msg::Quaternion createQuaternionFromYaw(double yaw)
{
  tf2::Quaternion tf_quat;
  tf_quat.setRPY(0.0, 0.0, yaw);

  return tf2::toMsg(tf_quat);
}

double rad2deg(double radians) { return radians * RAD2DEG; }

double deg2rad(double degrees) { return degrees * DEG2RAD; }

QImage toQImage(const data::MapData& map_data)
{
  const auto& theme = ROBOGait::settings::AppTheme::getInstance();
  const auto* map_theme = qobject_cast<const ROBOGait::settings::ThemeMap*>(theme.getMap());
  const QRgb map_unknown_color = map_theme ? map_theme->getMapUnknown().rgb() : QColor("#1a3a4a").rgb();
  const QRgb map_free_color = map_theme ? map_theme->getMapFree().rgb() : QColor("#a9cfe8").rgb();
  const QRgb map_occupied_color = map_theme ? map_theme->getMapOccupied().rgb() : QColor("#ffffff").rgb();

  if (!map_data.isAvailable())
  {
    qWarning() << "[utils::toQImage] Map data not available, cannot convert to QImage";
    return QImage();
  }

  const auto metadata = map_data.getMetadata();
  const auto& occupancy_data = map_data.getOccupancyData();

  const uint32_t width = metadata.width_;
  const uint32_t height = metadata.height_;

  if (width == 0 || height == 0)
  {
    qCritical() << "[utils::toQImage] Invalid map dimensions";
    return QImage();
  }

  const size_t expected_size = static_cast<size_t>(width) * static_cast<size_t>(height);

  if (occupancy_data.size() != expected_size)
  {
    qCritical() << "[utils::toQImage] Mismatched occupancy data size";
    return QImage();
  }

  QImage image(static_cast<int>(width), static_cast<int>(height), QImage::Format_RGB888);

  if (image.isNull())
  {
    qCritical() << "[utils::toQImage] Failed to create QImage";
    return QImage();
  }

  for (uint32_t y = 0; y < height; ++y)
  {
    for (uint32_t x = 0; x < width; ++x)
    {
      const uint32_t index = y * width + x;
      const int8_t occupancy = occupancy_data[index];

      QRgb color;

      if (occupancy == UNKNOWN_OCCUPANCY)
      {
        // Unknown: dark blue-gray
        color = map_unknown_color;
      }
      else if (occupancy < FREE_SPACE_THRESHOLD)
      {
        // Free space: light blue
        color = map_free_color;
      }
      else
      {
        // Occupied: white
        color = map_occupied_color;
      }

      image.setPixel(static_cast<int>(x), static_cast<int>(y), color);
    }
  }

  return image;
}

bool generateMapPreview(const data::MapData& map_data, const QString& map_name)
{
  if (!map_data.isAvailable())
  {
    qCritical() << "[utils::generateMapPreview] Invalid or unavailable map data";
    return false;
  }

  if (map_name.isEmpty())
  {
    qCritical() << "[utils::generateMapPreview] Map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qWarning() << "[utils::generateMapPreview] YAML loader is not loaded";
    return false;
  }

  const std::string image_path = yaml_loader.getValue<std::string>("map.image_path", "");

  const QString maps_directory = QDir::homePath() + "/" + QString::fromStdString(image_path);

  QDir dir;
  if (!dir.exists(maps_directory))
  {
    if (!dir.mkpath(maps_directory))
    {
      qCritical() << "[utils::generateMapPreview] Failed to create maps directory";
      return false;
    }
    qInfo() << "[utils::generateMapPreview] Created maps directory:" << maps_directory;
  }

  QString file_path = maps_directory + map_name + ".png";

  QImage image = toQImage(map_data);

  if (image.isNull())
  {
    qCritical() << "[utils::generateMapPreview] Failed to convert map data to QImage";
    return false;
  }

  image = image.mirrored(false, true);

  if (!image.save(file_path, "PNG", IMAGE_QUALITY))
  {
    qCritical() << "[utils::generateMapPreview] Failed to save map preview to file:" << file_path;
    return false;
  }

  qInfo() << "[utils::generateMapPreview] Map preview generated successfully at:" << file_path;

  return true;
}

bool deleteMapPreview(const QString& map_name)
{
  if (map_name.isEmpty())
  {
    qWarning() << "[utils::deleteMapPreview] Map name is empty";
    return false;
  }

  QString file_path = getMapPreviewPath(map_name);
  if (file_path.isEmpty())
  {
    qWarning() << "[utils::deleteMapPreview] Failed to get map preview path for:" << map_name;
    return false;
  }

  if (file_path.startsWith("file://"))
  {
    file_path = QUrl(file_path).toLocalFile();
  }

  if (!QFile::remove(file_path))
  {
    qWarning() << "[utils::deleteMapPreview] Failed to delete map preview at:" << file_path;
    return false;
  }

  qInfo() << "[utils::deleteMapPreview] Map preview deleted successfully at:" << file_path;

  return true;
}

QString getMapPreviewPath(const QString& map_name)
{
  if (map_name.isEmpty())
  {
    qWarning() << "[utils::getMapPreviewPath] Map name is empty";
    return QString();
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  if (!yaml_loader.isLoaded())
  {
    qWarning() << "[utils::getMapPreviewPath] YAML loader is not loaded";
    return QString();
  }

  std::string image_path = yaml_loader.getValue<std::string>("map.image_path", "");
  QString maps_directory = QDir::homePath() + "/" + QString::fromStdString(image_path);

  QString file_path = maps_directory + map_name + ".png";

  QFileInfo file_info(file_path);
  if (!file_info.exists())
  {
    qWarning() << "[utils::getMapPreviewPath] Map preview file does not exist:" << file_path;
    return QString();
  }

  return "file://" + file_path;
}

std::string sanitizeMapName(const std::string& map_name)
{
  if (map_name.empty())
  {
    qWarning() << "[utils::sanitizeMapName] Map name is empty";
    return std::string();
  }

  std::string out;
  out.reserve(map_name.size());

  bool last_was_separator = true;

  for (const char ch : map_name)
  {
    const unsigned char unsigned_ch = static_cast<unsigned char>(ch);
    const bool is_space = std::isspace(unsigned_ch) != 0;
    const bool is_allowed = std::isalnum(unsigned_ch) != 0 || ch == '-' || ch == '_';

    if (is_space || !is_allowed)
    {
      if (!last_was_separator)
      {
        out.push_back('_');
        last_was_separator = true;
      }
      continue;
    }

    out.push_back(static_cast<char>(ch));
    last_was_separator = false;
  }

  if (!out.empty() && out.back() == '_')
  {
    out.pop_back();
  }

  return out;
}

QVariantList toWaypointVariantList(const std::vector<WaypointInput>& points)
{
  QVariantList points_list;
  points_list.reserve(static_cast<int>(points.size()));

  for (const auto& waypoint : points)
  {
    QVariantMap point_map;
    point_map["x"] = waypoint.x;
    point_map["y"] = waypoint.y;
    if (waypoint.theta.has_value())
    {
      point_map["theta"] = waypoint.theta.value();
    }
    points_list.append(point_map);
  }

  return points_list;
}

std::vector<WaypointInput> parseWaypointInputs(const QVariantList& points)
{
  std::vector<WaypointInput> out;
  out.reserve(static_cast<size_t>(points.size()));

  for (const QVariant& value : points)
  {
    if (!value.canConvert<QVariantMap>())
    {
      continue;
    }

    const QVariantMap point_map = value.toMap();

    bool ok_x = false;
    bool ok_y = false;
    bool ok_theta = false;

    const double x = point_map.value("x").toDouble(&ok_x);
    const double y = point_map.value("y").toDouble(&ok_y);
    const double theta = point_map.value("theta").toDouble(&ok_theta);

    if (!ok_x || !ok_y || !std::isfinite(x) || !std::isfinite(y))
    {
      continue;
    }

    WaypointInput waypoint;
    waypoint.x = x;
    waypoint.y = y;

    if (ok_theta && std::isfinite(theta))
    {
      waypoint.theta = theta;
    }

    out.push_back(std::move(waypoint));
  }

  return out;
}

double getFollowPathMinInitialSpacingM()
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  if (!yaml_loader.isLoaded())
  {
    return DEFAULT_FOLLOW_PATH_MIN_INITIAL_SPACING_M;
  }

  const double configured_spacing =
      yaml_loader.getValue<double>("map.spline_path.follow_path_min_initial_spacing_m", DEFAULT_FOLLOW_PATH_MIN_INITIAL_SPACING_M);
  if (!std::isfinite(configured_spacing) || configured_spacing < 0.0)
  {
    qWarning() << "[utils::getFollowPathMinInitialSpacingM] Invalid follow_path_min_initial_spacing_m, using default:"
               << DEFAULT_FOLLOW_PATH_MIN_INITIAL_SPACING_M;
    return DEFAULT_FOLLOW_PATH_MIN_INITIAL_SPACING_M;
  }

  return configured_spacing;
}

double getFollowPathMinPointSpacingM()
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  if (!yaml_loader.isLoaded())
  {
    return DEFAULT_FOLLOW_PATH_MIN_POINT_SPACING_M;
  }

  const double configured_spacing = yaml_loader.getValue<double>("map.spline_path.follow_path_min_point_spacing_m", DEFAULT_FOLLOW_PATH_MIN_POINT_SPACING_M);
  if (!std::isfinite(configured_spacing) || configured_spacing < 0.0)
  {
    qWarning() << "[utils::getFollowPathMinPointSpacingM] Invalid follow_path_min_point_spacing_m, using default:" << DEFAULT_FOLLOW_PATH_MIN_POINT_SPACING_M;
    return DEFAULT_FOLLOW_PATH_MIN_POINT_SPACING_M;
  }

  return configured_spacing;
}

std::vector<WaypointInput> normalizeFollowPathWaypoints(const std::vector<WaypointInput>& waypoints, const WaypointInput& robot_pose,
                                                        double min_initial_spacing_m)
{
  if (waypoints.size() < 2)
  {
    return waypoints;
  }

  std::vector<WaypointInput> ordered_waypoints = waypoints;
  const double first_distance_squared = distanceSquared(robot_pose, ordered_waypoints.front());
  const double last_distance_squared = distanceSquared(robot_pose, ordered_waypoints.back());
  const bool should_reverse_path = last_distance_squared < first_distance_squared;
  const std::optional<double> terminal_theta = should_reverse_path ? ordered_waypoints.front().theta : ordered_waypoints.back().theta;

  if (should_reverse_path)
  {
    std::reverse(ordered_waypoints.begin(), ordered_waypoints.end());
  }

  const double min_spacing = std::max(0.0, min_initial_spacing_m);
  const double min_spacing_squared = min_spacing * min_spacing;
  const double min_point_spacing = std::max(0.0, getFollowPathMinPointSpacingM());
  const double min_point_spacing_squared = min_point_spacing * min_point_spacing;

  std::vector<WaypointInput> normalized_waypoints;
  normalized_waypoints.reserve(ordered_waypoints.size() + 1U);
  normalized_waypoints.push_back(robot_pose);

  for (size_t i = 0; i < ordered_waypoints.size(); ++i)
  {
    const auto& waypoint = ordered_waypoints[i];
    const bool is_final_input_point = i + 1U == ordered_waypoints.size();

    if (distanceSquared(robot_pose, waypoint) < min_spacing_squared)
    {
      continue;
    }

    if (!normalized_waypoints.empty() && distanceSquared(normalized_waypoints.back(), waypoint) <= 1e-9)
    {
      continue;
    }

    if (!is_final_input_point && distanceSquared(normalized_waypoints.back(), waypoint) < min_point_spacing_squared)
    {
      continue;
    }

    normalized_waypoints.push_back(waypoint);
  }

  if (normalized_waypoints.size() < 2U)
  {
    const auto& fallback_target = ordered_waypoints.back();
    if (distanceSquared(robot_pose, fallback_target) > 1e-9)
    {
      normalized_waypoints.push_back(fallback_target);
    }
  }

  if (normalized_waypoints.size() < 2U)
  {
    return normalized_waypoints;
  }

  for (size_t i = 0; i < normalized_waypoints.size(); ++i)
  {
    const bool is_final_point = i + 1U == normalized_waypoints.size();
    if (is_final_point && terminal_theta.has_value())
    {
      normalized_waypoints[i].theta = terminal_theta;
      continue;
    }

    normalized_waypoints[i].theta = calculateWaypointYaw(normalized_waypoints, i);
  }

  return normalized_waypoints;
}

geometry_msgs::msg::Quaternion buildWaypointOrientation(const WaypointInput& waypoint)
{
  if (waypoint.theta.has_value())
  {
    return createQuaternionFromYaw(waypoint.theta.value());
  }

  geometry_msgs::msg::Quaternion orientation;
  orientation.x = 0.0;
  orientation.y = 0.0;
  orientation.z = 0.0;
  orientation.w = 1.0;
  return orientation;
}

} // namespace utils
} // namespace map
} // namespace ROBOGait
