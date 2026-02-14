#pragma once

#include <QImage>
#include <QQuickImageProvider>

#include "Map/MapManager.hpp"

namespace ROBOGait
{
namespace map
{
namespace provider
{

/**
 * @brief QML Image Provider for map visualization
 */
class MapImageProvider : public QQuickImageProvider
{
public:
  /**
   * @brief Constructor of MapImageProvider
   */
  MapImageProvider();

  /**
   * @brief Destructor of MapImageProvider
   */
  ~MapImageProvider() override;

  /**
   * @brief Request image from MapManager
   *
   * @param id Image identifier (unused, map is always the same)
   * @param size Output parameter for image size
   * @param requestedSize Requested size (unused, returns actual map size)
   * @return QImage from MapManager or empty image if not available
   */
  QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
};

} // namespace provider
} // namespace map
} // namespace ROBOGait
