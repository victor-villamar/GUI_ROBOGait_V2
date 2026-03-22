#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

#include <rclcpp/callback_group.hpp>
#include <rclcpp/client.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/timer.hpp>

#include <command_executor_msgs/srv/cmd.hpp>
#include <command_executor_msgs/srv/get_map_data.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>

#include "Context/RobotContext.hpp"

namespace ROBOGait
{
namespace ros
{
namespace executor
{
/**
 * @brief Client class to execute commands on the ROS2 side
 *
 * This class manages the execution of commands. It interacts with a ROS2 service to send command
 * requests and monitors the health of the nodes associated with the commands.
 */
class CommandExecutorClient
{
public:
  /**
   * @brief Enum to represent the status of a command
   */
  enum class CommandStatus
  {
    IDLE,     /**< The command is idle */
    STARTING, /**< The command is starting */
    RUNNING,  /**< The command is running */
    STOPPING, /**< The command is stopping */
    STOPPED,  /**< The command has stopped */
    ERROR     /**< There was an error with the command */
  };

  /**
   * @brief Struct to hold command information
   *
   * @param name The name of the ROS2 node
   * @param cmd The command to be executed
   * @param append_args Additional arguments to append to the command
   */
  struct CommandInfo
  {
    std::string name;
    std::string cmd;
    std::string append_args;
  };

  /**
   * @brief Struct to hold the state of a command
   *
   * @param status The current status of the command
   * @param timestamp The timestamp when the command state was last updated
   * @param full_cmd The full command string that was executed
   */
  struct CommandState
  {
    CommandStatus status;
    std::chrono::steady_clock::time_point timestamp;
    std::string full_cmd;
  };

  /**
   * @brief Get the singleton instance of the CommandExecutorClient
   *
   * @return The singleton instance
   */
  static CommandExecutorClient& getInstance();

  /** Delete copy constructor and assignment operator */
  CommandExecutorClient(const CommandExecutorClient&) = delete;
  CommandExecutorClient& operator=(const CommandExecutorClient&) = delete;

  /**
   * @brief Initialize the CommandExecutorClient
   *
   * @param parent_node The parent ROS2 node
   *
   * @return true if initialization was successful, false otherwise
   */
  bool initialize(rclcpp::Node* parent_node);

  /**
   * @brief Check if the CommandExecutorClient is initialized
   *
   * @return true if the client is initialized, false otherwise
   */
  bool isInitialized() const;

  /**
   * @brief Set the robot context
   *
   * @param context The robot context to set
   */
  void setRobotContext(const ROBOGait::context::RobotContext& context);

  /**
   * @brief Clear the robot context
   */
  void clearRobotContext();

  /**
   * @brief Start the mapping process
   *
   * @return true if the mapping process was started successfully, false otherwise
   */
  bool startMapping();

  /**
   * @brief Stop the mapping process
   *
   * @param save_map Whether to save the map
   * @param map_name The name of the map to save
   *
   * @return true if the mapping process was stopped successfully, false otherwise
   */
  bool stopMapping(bool save_map, const std::string& map_name);

  /**
   * @brief Delete a map
   *
   * @param map_name The name of the map to delete
   *
   * @return true if the map was deleted successfully, false otherwise
   */
  bool deleteMap(const std::string& map_name);

  /**
   * @brief Request the map data for a given map name
   *
   * @param map_name The name of the map to request data for
   *
   * @return true if the map data was requested successfully, false otherwise
   */
  bool requestMapData(const std::string& map_name);

  /**
   * @brief Start the navigation process
   *
   * @param map_name The name of the map to use for navigation
   *
   * @return true if the navigation process was started successfully, false otherwise
   */
  bool startNavigation(const std::string& map_name);

  /**
   * @brief Stop the navigation process
   *
   * @return true if the navigation process was stopped successfully, false otherwise
   */
  bool stopNavigation();

  /**
   * @brief Check if a ROS2 node is alive
   *
   * @param node_name The name of the ROS2 node
   *
   * @return true if the node is alive, false otherwise
   */
  bool isNodeAlive(const std::string& node_name) const;

  /**
   * @brief Get the status of the active command
   *
   * @return The status of the active command
   */
  CommandStatus getActiveCommandStatus() const;

  /**
   * @brief Get the key of the active command
   *
   * @return The key of the active command
   */
  std::string getActiveCommandKey() const;

  /**
   * @brief Get the status of a command
   *
   * @param key The key of the command
   *
   * @return The status of the command
   */
  CommandStatus getCommandStatus(const std::string& key) const;

private:
  /**
   * @brief Constructor of the CommandExecutorClient class
   */
  CommandExecutorClient();
  ~CommandExecutorClient() = default;

  /**
   * @brief Load commands from the YAML configuration file
   *
   * @return true if commands were loaded successfully, false otherwise
   */
  bool loadCommands();

  /**
   * @brief Call a command service
   *
   * @param cmd The command to call
   * @param execute Whether to execute the command
   *
   * @return true if the service call was successful, false otherwise
   */
  bool callCommandService(const std::string& cmd, bool execute);

  /**
   * @brief Call the get map data service to retrieve YAML and PGM info for a map
   *
   * @param map_name The name of the map
   * @param yaml_out Output parameter to hold the retrieved YAML info (if successful)
   * @param pgm_out Output parameter to hold the retrieved PGM info (if successful)
   *
   * @return true if the service call was successful and data was retrieved, false otherwise
   */
  bool callGetMapDataService(const std::string& map_name, std::optional<std::string>& yaml_out, std::optional<std::vector<uint8_t>>& pgm_out);

  /**
   * @brief Build a command string
   *
   * @param cmd The command to build
   * @param args The arguments to append to the command
   *
   * @return The full command string
   */
  std::string buildCommand(const std::string& cmd, const std::string& args);

  /**
   * @brief Validate a command key
   *
   * @param key The command key to validate
   *
   * @return true if the command key is valid, false otherwise
   */
  bool validateCommandKey(const std::string& key) const;

  /**
   * @brief Resolve the service name for the command executor
   *
   * Use the robot context to resolve the service name with namespace or without
   *
   * @param service_name The base service name to resolve
   *
   * @return The resolved service name
   */
  std::string resolveServiceName(const std::string& service_name) const;

  /**
   * @brief Rebuild the command service client
   *
   * @return true if the client was rebuilt successfully, false otherwise
   */
  bool rebuildClient();

  /**
   * @brief Save the current map
   *
   * @param map_name The name of the map to save
   *
   * @return true if the map was saved successfully, false otherwise
   */
  bool saveMap(const std::string& map_name);

  /**
   * @brief Stop a command
   *
   * @param key The key of the command to stop
   *
   * @return true if the command was stopped successfully, false otherwise
   */
  bool stopCommand(const std::string& key);

  /**
   * @brief Set the state of a command
   *
   * @param key The key of the command
   * @param status The status to set
   * @param full_cmd The full command string
   */
  void setCommandState(const std::string& key, CommandStatus status, const std::string& full_cmd = std::string());

  /**
   * @brief Check if a command status is considered active.
   */
  bool isCommandActive(CommandStatus status) const;

  /**
   * @brief Start the health timer
   */
  void startHealthTimer();

  /**
   * @brief Stop the health timer
   */
  void stopHealthTimer();

  /**
   * @brief Callback for the health timer
   */
  void onHealthTimer();

  /**
   * @brief Replace placeholders in a string
   *
   * @param input The input string
   * @param values The map of placeholder values. Keys are the placeholders to replace, values are the corresponding replacement strings.
   *
   * @return The modified input string
   */
  std::string replacePlaceholders(std::string input, const std::unordered_map<std::string, std::string>& values) const;

  /**
   * @brief Publish map data once
   *
   * @param occupancy_grid The occupancy grid to publish
   *
   * @return true if the data was published successfully, false otherwise
   */
  bool publishMapDataOnce(const nav_msgs::msg::OccupancyGrid& occupancy_grid);

  rclcpp::Node* parent_node_;                                                          /**< The parent ROS2 node */
  rclcpp::Client<command_executor_msgs::srv::Cmd>::SharedPtr cli_cmd_;                 /**< The command service client */
  rclcpp::Client<command_executor_msgs::srv::GetMapData>::SharedPtr cli_get_map_data_; /**< The get map data service client */
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr pub_map_data_;            /**< Publisher for map data */
  rclcpp::CallbackGroup::SharedPtr cb_group_;                                          /**< The callback group for the command executor */
  rclcpp::TimerBase::SharedPtr timer_health_;                                          /**< The health timer */

  std::optional<ROBOGait::context::RobotContext> context_; /**< The robot context */

  std::unordered_map<std::string, CommandInfo> commands_; /**< Map of command names to their info. Keys are command names, values are CommandInfo structs. */
  std::unordered_map<std::string, CommandState>
      command_states_; /**< Map of command names to their states. Keys are command names, values are CommandState structs. */

  bool initialized_;              /**< Flag indicating if the client is initialized */
  bool pending_stop_after_save_;  /**< Flag indicating if a stop is pending after save */
  bool map_saver_stop_requested_; /**< Flag indicating if a stop is requested for the map saver */

  static constexpr const char* KEY_CARTOGRAPHER = "cartographer"; /**< Key for the cartographer command */
  static constexpr const char* KEY_MAP_SAVER = "map_saver";       /**< Key for the map saver command */
  static constexpr const char* KEY_DELETE_MAP = "delete_map";     /**< Key for the delete map command */
  static constexpr const char* KEY_NAVIGATION = "navigation";     /**< Key for the navigation command */

  static constexpr std::chrono::milliseconds HEALTH_CHECK_PERIOD = std::chrono::milliseconds(100);                 /**< Health check period */
  static constexpr std::chrono::seconds START_STOP_TIMEOUT = std::chrono::seconds(5);                              /**< Start/stop timeout */
  static constexpr std::chrono::milliseconds MAP_SAVER_FORCE_STOP_DELAY = std::chrono::milliseconds(2000);         /**< Map saver force stop delay */
  static constexpr std::chrono::milliseconds MAP_SAVER_STARTUP_ASSUME_STOP_DELAY = std::chrono::milliseconds(500); /**< Map saver startup assume stop delay */
};
} // namespace executor
} // namespace ros
} // namespace ROBOGait
