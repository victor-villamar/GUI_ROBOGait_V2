#pragma once

#include <memory>

#include <rclcpp/node.hpp>

#include "Context/RobotContext.hpp"
#include "Map/Data/MapData.hpp"
#include "Map/Source/SourceInterface.hpp"
#include "Map/Subscribers/MapSubscriber.hpp"

namespace ROBOGait
{
namespace map
{
namespace source
{

/**
 * @brief Owns MapData and wires it to ROS map subscriptions
 */
class MapSource : public SourceInterface
{
public:
  MapSource();

  /**
   * @brief Initializes the map source.
   *
   * @param parent_node The parent ROS node.
   */
  SourceResult initialize(rclcpp::Node* parent_node) override;

  /**
   * @brief Sets the robot context.
   *
   * @param context The robot context.
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context) override;

  void start() override;
  void stop() override;

  bool isActive() const override;
  bool isAvailable() const override;

  std::shared_ptr<ROBOGait::map::data::MapData> getMapData() const;

private:
  std::shared_ptr<ROBOGait::map::data::MapData> map_data_;                /**< Map data */
  std::shared_ptr<ROBOGait::map::subscribers::MapSubscriber> subscriber_; /**< Map subscriber */
};

} // namespace source
} // namespace map
} // namespace ROBOGait
