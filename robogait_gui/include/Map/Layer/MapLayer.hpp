#pragma once

#include <memory>
#include <string>

#include <QImage>
#include <QMutex>

#include "Map/Data/MapData.hpp"
#include "Map/Layer/LayerInterface.hpp"

namespace ROBOGait
{
namespace map
{
namespace layer
{

/**
 * @brief Layer that caches and manages map data
 */
class MapLayer : public LayerInterface
{
public:
  explicit MapLayer(std::shared_ptr<data::MapData> map_data = nullptr);

  std::shared_ptr<data::MapData> getMapData() const;

  /**
   * @brief Get the cached map image
   */
  const QImage& getImage() const;

  /**
   * @brief Update the renderer state
   */
  void update() override;

  /**
   * @brief Check if the renderer needs to be re-rendered
   */
  bool needsRender() const override;

  /**
   * @brief Clear the render request flag
   */
  void clearRenderRequest() override;

private:
  /**
   * @brief Refresh the cached map image
   */
  void refreshImage();

  std::shared_ptr<data::MapData> map_data_; /**< Map data source */
  mutable QMutex image_mutex_;              /**< Mutex for protecting access to the cached image */
  QImage cached_image_;                     /**< Cached map image */
  uint64_t last_stamp_;                     /**< Last update stamp */
  bool render_requested_;                   /**< Flag indicating if rendering is requested */
};

} // namespace layer
} // namespace map
} // namespace ROBOGait
