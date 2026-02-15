#include <QDebug>

#include "Map/MapImageProvider.hpp"

using namespace ROBOGait::map::provider;

MapImageProvider::MapImageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{
  qInfo() << "[MapImageProvider::MapImageProvider] Map image provider created";
}

MapImageProvider::~MapImageProvider() { qInfo() << "[MapImageProvider::~MapImageProvider] Map image provider destroyed"; }

QImage MapImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
  // Unused parameter, needed for QQuickImageProvider
  Q_UNUSED(requestedSize);

  auto& map_manager = ROBOGait::map::manager::MapManager::getInstance();

  if (!map_manager.isInitialized() || !map_manager.isMapAvailable())
  {
    qWarning() << "[MapImageProvider::requestImage] Map not available for id:" << id;
    if (size)
    {
      *size = QSize(0, 0);
    }
    return QImage();
  }

  QImage map_image = map_manager.getMapImage();

  if (size)
  {
    *size = map_image.size();
  }

  // DEBUG
  // qDebug() << "[MapImageProvider::requestImage] Providing map image of size:" << map_image.size() << "for id:" << id;

  return map_image;
}
