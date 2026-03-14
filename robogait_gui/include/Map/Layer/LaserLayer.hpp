#pragma once

#include <memory>
#include <vector>

#include <QMutex>
#include <QPointF>

#include "Map/Data/LaserScanData.hpp"
#include "Map/Layer/LayerInterface.hpp"

namespace ROBOGait
{
namespace map
{
namespace layer
{
/**
 * Layer that caches and manages laser scan data
 */
class LaserLayer : public LayerInterface
{
public:
  /**
   * @brief Constructor for the LaserLayer class
   */
  explicit LaserLayer();

  /**
   * @brief Set the laser scan data
   *
   * @param scan_data Shared pointer to the laser scan data
   */
  void setLaserScanData(std::shared_ptr<data::LaserScanData> scan_data);

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

  /**
   * @brief Get the points to be rendered
   */
  std::vector<QPointF> getPoints() const;

private:
  std::shared_ptr<data::LaserScanData> scan_data_; /**< Laser scan data source */
  mutable QMutex data_mutex_;                      /**< Mutex for thread-safe access to data */
  std::vector<QPointF> cached_points_;             /**< Cached points for rendering */
  uint64_t last_stamp_;                            /**< Timestamp of the last update */
  bool render_requested_;                          /**< Flag indicating if rendering is requested */
};

} // namespace layer
} // namespace map
} // namespace ROBOGait
