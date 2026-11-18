#pragma once

#include <memory>
#include <vector>

#include <QMutex>

#include "Map/Data/PathData.hpp"
#include "Map/Layer/LayerInterface.hpp"

namespace ROBOGait
{
namespace map
{
namespace layer
{
/**
 * @brief Layer that caches and manages path data
 */
class PathLayer : public LayerInterface
{
public:
  /**
   * @brief Constructor for the PathLayer class
   */
  explicit PathLayer();

  /**
   * @brief Set the path data
   *
   * @param path_data Shared pointer to the path data
   */
  void setPathData(std::shared_ptr<data::PathData> path_data);

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
  std::vector<data::PathData::PathPoint> getPoints() const;

private:
  std::shared_ptr<data::PathData> path_data_;            /**< Path data source */
  mutable QMutex data_mutex_;                            /**< Mutex for thread-safe access to data */
  std::vector<data::PathData::PathPoint> cached_points_; /**< Cached points for rendering */
  uint64_t last_stamp_;                                  /**< Timestamp of the last update */
  bool render_requested_;                                /**< Flag indicating if rendering is requested */
};

} // namespace layer
} // namespace map
} // namespace ROBOGait
