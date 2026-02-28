#pragma once

#include <memory>

#include "Map/Layer/MapLayer.hpp"
#include "Map/Layer/RobotLayer.hpp"
#include "Map/Rendering/RenderPipeline.hpp"

namespace ROBOGait
{
namespace map
{
namespace rendering
{

/**
 * @brief Scene container that owns the pipeline and registered layers
 *
 * Manages the rendering pipeline and the layers state to be rendered
 */
class RenderScene
{
public:
  RenderScene();

  void setMapLayer(std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer);
  void setRobotLayer(std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer);

  std::shared_ptr<RenderPipeline> getPipeline() const;

  /**
   * @brief Start the rendering process
   */
  void start();

  /**
   * @brief Stop the rendering process
   */
  void stop();

private:
  std::shared_ptr<RenderPipeline> pipeline_;                      /**< Render pipeline */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer_;     /**< Map layer */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer_; /**< Robot layer */
};

} // namespace rendering
} // namespace map
} // namespace ROBOGait
