#pragma once

#include <memory>

#include "Map/Layer/LaserLayer.hpp"
#include "Map/Layer/MapLayer.hpp"
#include "Map/Layer/ParticleCloudLayer.hpp"
#include "Map/Layer/PathLayer.hpp"
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
  /**
   * @brief Constructor for the RenderScene class
   */
  RenderScene();

  /**
   * @brief Set the map layer for the render scene
   *
   * @param map_layer The map layer to set
   */
  void setMapLayer(std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer);

  /**
   * @brief Set the robot layer for the render scene
   *
   * @param robot_layer The robot layer to set
   */
  void setRobotLayer(std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer);

  /**
   * @brief Set the laser layer for the render scene
   *
   * @param laser_layer The laser layer to set
   */
  void setLaserLayer(std::shared_ptr<ROBOGait::map::layer::LaserLayer> laser_layer);

  /**
   * @brief Set the particle cloud layer for the render scene
   *
   * @param particle_layer The particle cloud layer to set
   */
  void setParticleCloudLayer(std::shared_ptr<ROBOGait::map::layer::ParticleCloudLayer> particle_layer);

  /**
   * @brief Set the path layer for the render scene
   *
   * @param path_layer The path layer to set
   */
  void setPathLayer(std::shared_ptr<ROBOGait::map::layer::PathLayer> path_layer);

  /**
   * @brief Get the render pipeline for the render scene
   *
   * @return The render pipeline
   */
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
  std::shared_ptr<RenderPipeline> pipeline_;                                 /**< Render pipeline */
  std::shared_ptr<ROBOGait::map::layer::MapLayer> map_layer_;                /**< Map layer */
  std::shared_ptr<ROBOGait::map::layer::RobotLayer> robot_layer_;            /**< Robot layer */
  std::shared_ptr<ROBOGait::map::layer::LaserLayer> laser_layer_;            /**< Laser layer */
  std::shared_ptr<ROBOGait::map::layer::ParticleCloudLayer> particle_layer_; /**< Particle cloud layer */
  std::shared_ptr<ROBOGait::map::layer::PathLayer> path_layer_;              /**< Path layer */
};

} // namespace rendering
} // namespace map
} // namespace ROBOGait
