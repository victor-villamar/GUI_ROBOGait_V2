#pragma once

#include <memory>

#include "Map/Data/PathData.hpp"
#include "Map/Source/SourceInterface.hpp"
#include "Map/Subscribers/PathSubscriber.hpp"

namespace ROBOGait
{
namespace map
{
namespace source
{
/**
 * @brief Owns PathData and wires it to ROS path subscriptions
 */
class PathSource : public SourceInterface
{
public:
  /**
   * @brief Constructor for the PathSource class
   */
  PathSource();

  /**
   * @brief Initialize the path source
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
   * @brief Start the path source
   */
  void start() override;

  /**
   * @brief Stop the path source
   */
  void stop() override;

  /**
   * @brief Check if the path source is active
   *
   * @return True if active, false otherwise
   */
  bool isActive() const override;

  /**
   * @brief Check if the path source is available
   *
   * @return True if available, false otherwise
   */
  bool isAvailable() const override;

  /**
   * @brief Get the path data
   *
   * @return The PathData instance
   */
  std::shared_ptr<ROBOGait::map::data::PathData> getPathData() const;

private:
  std::shared_ptr<ROBOGait::map::data::PathData> path_data_;               /**< Path data */
  std::shared_ptr<ROBOGait::map::subscribers::PathSubscriber> subscriber_; /**< Path subscriber */
};

} // namespace source
} // namespace map
} // namespace ROBOGait
