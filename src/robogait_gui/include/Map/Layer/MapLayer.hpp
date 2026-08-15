#pragma once

#include <memory>

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
  /**
   * @brief Constructor of MapLayer class
   */
  explicit MapLayer();

  /**
   * @brief Set the map data
   *
   * @param map_data Shared pointer to the map data
   */
  void setMapData(std::shared_ptr<data::MapData> map_data);

  /**
   * @brief Get the map data
   *
   * @return Shared pointer to the map data
   */
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
