#pragma once

#include <memory>

#include "Map/Data/ParticleCloudData.hpp"
#include "Map/Source/SourceInterface.hpp"
#include "Map/Subscribers/ParticleCloudSubscriber.hpp"

namespace ROBOGait
{
namespace map
{
namespace source
{
/**
 * @brief Owns ParticleCloudData and wires it to ROS particle cloud subscriptions
 */
class ParticleCloudSource : public SourceInterface
{
public:
  /**
   * @brief Constructor for the ParticleCloudSource class
   */
  ParticleCloudSource();

  /**
   * @brief Initialize the particle cloud source
   *
   * @param parent_node The parent ROS node
   */
  SourceResult initialize(rclcpp::Node* parent_node) override;

  /**
   * @brief Set the robot context
   *
   * @param context The RobotContext instance
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context) override;

  /**
   * @brief Start the particle cloud source
   */
  void start() override;

  /**
   * @brief Stop the particle cloud source
   */
  void stop() override;

  /**
   * @brief Check if the particle cloud source is active
   *
   * @return True if active, false otherwise
   */
  bool isActive() const override;

  /**
   * @brief Check if the particle cloud source is available
   *
   * @return True if available, false otherwise
   */
  bool isAvailable() const override;

  /**
   * @brief Get the particle cloud data
   *
   * @return The ParticleCloudData instance
   */
  std::shared_ptr<ROBOGait::map::data::ParticleCloudData> getParticleCloudData() const;

private:
  std::shared_ptr<ROBOGait::map::data::ParticleCloudData> cloud_data_;              /**< Particle cloud data */
  std::shared_ptr<ROBOGait::map::subscribers::ParticleCloudSubscriber> subscriber_; /**< Particle cloud subscriber */
};

} // namespace source
} // namespace map
} // namespace ROBOGait
