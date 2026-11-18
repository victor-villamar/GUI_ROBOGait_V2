#pragma once

#include <optional>
#include <string>

#include <rclcpp/node.hpp>

#include "Context/RobotContext.hpp"

namespace ROBOGait
{
namespace map
{
namespace source
{

/**
 * @brief Interface for ROS-backed data sources
 */
class SourceInterface
{
public:
  /**
   * @brief Result returned by source control operations
   */
  struct SourceResult
  {
    static SourceResult success();
    static SourceResult failure(std::string error_in);

    std::string error;

    explicit operator bool() const;

  private:
    SourceResult(bool success_in, std::string error_in);

    bool success_;
  };

  virtual ~SourceInterface() = default;

  virtual SourceResult initialize(rclcpp::Node* parent_node) = 0;
  virtual void setRobotContext(const ROBOGait::context::RobotContext& context) = 0;

  virtual void start() = 0;
  virtual void stop() = 0;

  virtual bool isActive() const = 0;
  virtual bool isAvailable() const = 0;

protected:
  SourceInterface() = default;

  rclcpp::Node* parent_node_;                              /**< Parent node for subscriptions */
  std::optional<ROBOGait::context::RobotContext> context_; /**< Robot context */
  bool initialized_;                                       /**< Flag indicating if source is initialized */
  bool active_;                                            /**< Flag indicating if source is active */
};

} // namespace source
} // namespace map
} // namespace ROBOGait
