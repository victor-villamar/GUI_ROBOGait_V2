#pragma once

#include <memory>
#include <string>

#include <rclcpp/node.hpp>

#include "Context/RobotContext.hpp"
#include "Map/Data/RobotPoseData.hpp"
#include "Map/Source/SourceInterface.hpp"
#include "Map/Subscribers/TFSubscriber.hpp"

namespace ROBOGait
{
namespace map
{
namespace source
{

/**
 * @brief Owns RobotPoseData and controls TF-based pose streaming
 */
class RobotPoseSource : public SourceInterface
{
public:
  RobotPoseSource();

  /**
   * @brief Initializes the robot pose source.
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

  /**
   * @brief Pause or resume TF pose updates without resetting pose data
   */
  SourceResult setPaused(bool paused);

  bool isActive() const override;
  bool isAvailable() const override;

  std::shared_ptr<ROBOGait::map::data::RobotPoseData> getRobotPoseData() const;

private:
  std::shared_ptr<ROBOGait::map::data::RobotPoseData> robot_pose_data_;     /**< Robot pose data */
  std::shared_ptr<ROBOGait::map::subscribers::TFSubscriber> tf_subscriber_; /**< TF subscriber for pose updates */
  std::string map_frame_;                                                   /**< Map frame to use for TF lookups */
  std::string robot_frame_;                                                 /**< Robot frame to use for TF lookups */
};

} // namespace source
} // namespace map
} // namespace ROBOGait
