#pragma once

#include <memory>
#include <string>
#include <vector>

#include <rclcpp/node.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/service.hpp>
#include <rclcpp/timer.hpp>

#include <command_executor_msgs/msg/robot_status.hpp>
#include <command_executor_msgs/srv/cmd.hpp>

#include "ProcessManager.hpp"
#include "YamlLoader.hpp"

namespace ROBOGait
{
namespace command
{
class CommandExecutor : public rclcpp::Node
{
public:
  CommandExecutor();

private:
  /**
   * @brief Struct to hold information about the robot
   *
   * @param id Robot ID
   * @param battery Battery level (0.0 to 100.0)
   * @param ns Robot namespace
   * @param version Robot version
   * @param hardware_id Robot hardware ID
   * @param serial_number Robot serial number
   */
  struct RobotInfo
  {
    uint8_t id{0};
    float battery{0.0f};
    std::string ns;
    std::string version;
    uint32_t hardware_id{0};
    std::string serial_number;
  };

  void handleCommand(const std::shared_ptr<command_executor_msgs::srv::Cmd::Request>& request,
                     std::shared_ptr<command_executor_msgs::srv::Cmd::Response> response);
  void mainLoop();

  bool isAllowedCommand(const std::string& cmd) const;
  static std::string ltrimCopy(const std::string& value);
  void loadConfig();
  static std::string resolveConfigPath();

  rclcpp::Service<command_executor_msgs::srv::Cmd>::SharedPtr srv_cmd_;                    /**< Command service */
  rclcpp::Publisher<command_executor_msgs::msg::RobotStatus>::SharedPtr pub_robot_status_; /**< Robot status publisher */
  rclcpp::TimerBase::SharedPtr timer_;                                                     /**< Timer */

  std::vector<std::string> allow_list_; /**< List of allowed commands */
  RobotInfo robot_info_;                /**< Robot information */
  ProcessManager process_manager_;      /**< Process manager */
};
} // namespace command
} // namespace ROBOGait
