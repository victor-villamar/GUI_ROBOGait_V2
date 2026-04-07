#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

namespace ROBOGait
{
namespace map
{
namespace data
{
/**
 * @brief Class for handling particle cloud data
 */
class ParticleCloudData
{
public:
  /**
   * @brief Structure representing a single particle in the cloud
   *
   * @param x The x-coordinate of the particle
   * @param y The y-coordinate of the particle
   * @param theta The orientation of the particle in radians
   * @param weight The weight of the particle
   */
  struct Particle
  {
    double x;
    double y;
    double theta;
    double weight;

    Particle() : x(0.0), y(0.0), theta(0.0), weight(0.0) {}
    Particle(double x, double y, double theta, double weight) : x(x), y(y), theta(theta), weight(weight) {}
  };

  /**
   * @brief Particle Cloud Metadata
   *
   * @param particles The particles in the cloud
   */
  struct ParticleCloudMetadata
  {
    std::vector<Particle> particles;
  };

  /**
   * @brief Constructor of ParticleCloudData class
   */
  ParticleCloudData();

  /**
   * @brief Destructor of ParticleCloudData class
   */
  ~ParticleCloudData() = default;

  /**
   * @brief Set the particle cloud data
   *
   * @param metadata Particle cloud metadata
   */
  void setParticleCloudData(const ParticleCloudMetadata& metadata);

  /**
   * @brief Get the metadata from the particle cloud data
   *
   * @return The metadata from the particle cloud data
   */
  ParticleCloudMetadata getMetadata() const;

  /**
   * @brief Get the particles from the particle cloud data
   *
   * @return The particles from the particle cloud data
   */
  std::vector<Particle> getParticles() const;

  /**
   * @brief Check if the particle cloud data is available
   *
   * @return true if particles are available, false otherwise
   */
  bool isAvailable() const;

  /**
   * @brief Reset the particle cloud data
   */
  void reset();

  /**
   * @brief Get the timestamp of the last update
   *
   * @return The timestamp of the last update
   */
  uint64_t getUpdateStamp() const;

private:
  ParticleCloudMetadata metadata_; /**< The metadata from the particle cloud data */
  bool is_available_;              /**< Flag indicating if the data is available */
  uint64_t update_stamp_;          /**< The update timestamp */

  mutable std::mutex data_mutex_; /**< Mutex for protecting shared data */
};
} // namespace data
} // namespace map
} // namespace ROBOGait
