#pragma once

#include <memory>
#include <string>
#include <vector>

#include <rclcpp/node.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/service.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/timer.hpp>

#include <command_executor_msgs/msg/robot_status.hpp>
#include <command_executor_msgs/srv/cmd.hpp>
#include <command_executor_msgs/srv/get_map_data.hpp>
#include <sensor_msgs/msg/battery_state.hpp>

#include "ProcessManager.hpp"

namespace ROBOGait
{
namespace command
{
/**
 * @brief Class for executing specific ros2 commands on the robot
 */
class CommandExecutor : public rclcpp::Node
{
public:
  /**
   * @brief Constructor for the CommandExecutor class
   *
   * @param options Node options
   */
  CommandExecutor(const rclcpp::NodeOptions& options);

  /**
   * @brief Destructor for the CommandExecutor class
   */
  ~CommandExecutor();

  /**
   * @brief Initialize the command executor
   *
   * @return true if initialization was successful, false otherwise
   */
  bool initialize();

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

  /**
   * @brief Handle a command request
   *
   * @param request The command request
   * @param response The command response
   */
  void handleCommand(const std::shared_ptr<command_executor_msgs::srv::Cmd::Request>& request,
                     std::shared_ptr<command_executor_msgs::srv::Cmd::Response> response);

  /**
   * @brief Handle a get map data request
   *
   * @param request The get map data request
   * @param response The get map data response
   */
  void handleGetMapData(const std::shared_ptr<command_executor_msgs::srv::GetMapData::Request>& request,
                        std::shared_ptr<command_executor_msgs::srv::GetMapData::Response> response);

  /**
   * @brief Callback for battery status updates
   *
   * @param msg The battery status message
   */
  void callbackBatteryStatus(const sensor_msgs::msg::BatteryState::SharedPtr msg);

  /**
   * @brief Main loop for processing commands
   */
  void mainLoop();

  /**
   * @brief Check if a command is allowed
   *
   * @param cmd The command to check
   *
   * @return true if the command is allowed, false otherwise
   */
  bool isAllowedCommand(const std::string& cmd) const;

  /**
   * @brief Trim leading whitespace from a string
   *
   * @param value The string to trim
   *
   * @return A copy of the string with leading whitespace removed
   */
  static std::string ltrimCopy(const std::string& value);

  /**
   * @brief Load configuration from YAML file
   *
   * @return true if loading was successful, false otherwise
   */
  bool loadConfig();

  /**
   * @brief Resolve the configuration file path
   *
   * @return The resolved configuration file path
   */
  static std::string resolveConfigPath();

  /**
   * @brief Create ROS interfaces (service, publisher, subscriber)
   *
   * @return true if creation was successful, false otherwise
   */
  bool createRosInterfaces();

  /**
   * @brief Check if a command is a delete command
   *
   * @param cmd The command to check
   * @param translated_cmd The translated command (if it is a delete command)
   *
   * @return true if the command is a delete command, false otherwise
   */
  bool isDeleteCommand(const std::string& cmd, std::string& translated_cmd) const;

  rclcpp::Service<command_executor_msgs::srv::Cmd>::SharedPtr srv_cmd_;                    /**< Command service */
  rclcpp::Service<command_executor_msgs::srv::GetMapData>::SharedPtr srv_get_map_data_;    /**< Get map data service */
  rclcpp::Publisher<command_executor_msgs::msg::RobotStatus>::SharedPtr pub_robot_status_; /**< Robot status publisher */
  rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr sub_battery_status_;     /**< Battery status subscriber */
  rclcpp::TimerBase::SharedPtr timer_;                                                     /**< Timer */

  std::vector<std::string> allow_list_; /**< List of allowed commands */
  RobotInfo robot_info_;                /**< Robot information */
  ProcessManager process_manager_;      /**< Process manager */
};
} // namespace command
} // namespace ROBOGait
