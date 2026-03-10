#pragma once

#include <memory>

#include "Map/Data/LaserScanData.hpp"
#include "Map/Source/SourceInterface.hpp"
#include "Map/Subscribers/LaserScanSubscriber.hpp"

namespace ROBOGait
{
namespace map
{
namespace source
{
/**
 * @brief Owns LaserScanData and wires it to ROS laser scan subscriptions
 */
class LaserSource : public SourceInterface
{
public:
  /**
   * @brief Constructor for the LaserSource class
   */
  LaserSource();

  /**
   * @brief Initialize the laser source
   *
   * @param parent_node The parent ROS node
   */
  void initialize(rclcpp::Node* parent_node) override;

  /**
   * @brief Set the robot context
   *
   * @param context The RobotContext instance
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context) override;

  /**
   * @brief Start the laser source
   */
  void start() override;

  /**
   * @brief Stop the laser source
   */
  void stop() override;

  /**
   * @brief Check if the laser source is active
   *
   * @return True if active, false otherwise
   */
  bool isActive() const override;

  /**
   * @brief Check if the laser source is available
   *
   * @return True if available, false otherwise
   */
  bool isAvailable() const override;

  /**
   * @brief Get the laser scan data
   *
   * @return The LaserScanData instance
   */
  std::shared_ptr<ROBOGait::map::data::LaserScanData> getLaserScanData() const;

private:
  std::shared_ptr<ROBOGait::map::data::LaserScanData> scan_data_;        /**< Laser scan data */
  std::shared_ptr<ROBOGait::map::data::LaserScanSubscriber> subscriber_; /**< Laser scan subscriber */
};

} // namespace source
} // namespace map
} // namespace ROBOGait
