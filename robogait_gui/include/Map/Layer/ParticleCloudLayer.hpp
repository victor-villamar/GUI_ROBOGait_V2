#pragma once

#include <memory>
#include <vector>

#include <QMutex>

#include "Map/Data/ParticleCloudData.hpp"
#include "Map/Layer/LayerInterface.hpp"

namespace ROBOGait
{
namespace map
{
namespace layer
{
/**
 * Layer that caches and manages particle cloud data
 */
class ParticleCloudLayer : public LayerInterface
{
public:
  /**
   * @brief Constructor for the ParticleCloudLayer class
   */
  explicit ParticleCloudLayer();

  /**
   * @brief Set the particle cloud data
   *
   * @param cloud_data Shared pointer to the particle cloud data
   */
  void setParticleCloudData(std::shared_ptr<data::ParticleCloudData> cloud_data);

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
   * @brief Get the particles to be rendered
   */
  std::vector<data::ParticleCloudData::Particle> getParticles() const;

private:
  std::shared_ptr<data::ParticleCloudData> cloud_data_;             /**< Particle cloud data source */
  mutable QMutex data_mutex_;                                       /**< Mutex for thread-safe access to data */
  std::vector<data::ParticleCloudData::Particle> cached_particles_; /**< Cached particles for rendering */
  uint64_t last_stamp_;                                             /**< Timestamp of the last update */
  bool render_requested_;                                           /**< Flag indicating if rendering is requested */
};

} // namespace layer
} // namespace map
} // namespace ROBOGait
