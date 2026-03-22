#include <cctype>
#include <cstdint>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>
#include <QUrl>
#include <Qt>

#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

#include "Loader/YamlLoader.hpp"
#include "Map/Utils/Utils.hpp"

namespace ROBOGait
{
namespace map
{
namespace utils
{
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

  if (!map_data.isAvailable())
  {
    qWarning() << "[utils::toQImage] Map data not available, cannot convert to QImage";
    return QImage();
  }

  const auto metadata = map_data.getMetadata();
  const auto& occupancy_data = map_data.getOccupancyData();

  const uint32_t width = metadata.width;
  const uint32_t height = metadata.height;

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

  QImage image(width, height, QImage::Format_RGB888);

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
        color = DARK_BLUE_GRAY;
      }
      else if (occupancy < FREE_SPACE_THRESHOLD)
      {
        // Free space: light blue
        color = LIGHT_BLUE;
      }
      else
      {
        // Occupied: white
        color = WHITE;
      }

      image.setPixel(x, y, color);
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

  image = image.flipped(Qt::Vertical);

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

  for (unsigned char ch : map_name)
  {
    const bool is_space = std::isspace(ch);
    const bool is_allowed = std::isalnum(ch) != 0 || ch == '-' || ch == '_';

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

} // namespace utils
} // namespace map
} // namespace ROBOGait
