#include <cmath>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QUrl>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>

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

double rad2deg(double radians) { return radians * (180.0 / M_PI); }

double deg2rad(double degrees) { return degrees * (M_PI / 180.0); }

bool generateMapPreview(const std::shared_ptr<data::MapData>& map_data, const QString& map_name)
{
  if (!map_data || !map_data->isAvailable())
  {
    qCritical() << "[utils::generateMapPreview] Invalid or unavailable map data";
    return false;
  }

  if (map_name.isEmpty())
  {
    qCritical() << "[utils::generateMapPreview] Map name is empty";
    return false;
  }

  std::string package_share_dir = ament_index_cpp::get_package_share_directory("robogait_gui");
  QString maps_directory = QString::fromStdString(package_share_dir) + "/maps";

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

  QString file_path = maps_directory + "/" + map_name + ".png";

  QImage image = map_data->toQImage();

  if (image.isNull())
  {
    qCritical() << "[utils::generateMapPreview] Failed to convert map data to QImage";
    return false;
  }

  if (!image.save(file_path, "PNG", 100))
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

  std::string package_share_dir = ament_index_cpp::get_package_share_directory("robogait_gui");
  QString file_path = QString::fromStdString(package_share_dir) + "/maps/" + map_name + ".png";

  QFileInfo file_info(file_path);
  if (!file_info.exists())
  {
    qWarning() << "[utils::getMapPreviewPath] Map preview file does not exist:" << file_path;
    return QString();
  }

  return "file://" + file_path;
}

} // namespace utils
} // namespace map
} // namespace ROBOGait
