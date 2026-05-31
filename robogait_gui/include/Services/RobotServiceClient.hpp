#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <QVariantList>

#include <rclcpp/callback_group.hpp>
#include <rclcpp/client.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/timer.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include <command_executor_msgs/srv/cmd.hpp>
#include <command_executor_msgs/srv/get_map_data.hpp>
#include <nav2_msgs/action/compute_path_through_poses.hpp>
#include <nav2_msgs/action/compute_path_to_pose.hpp>
#include <nav2_msgs/action/navigate_through_poses.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <nav_msgs/msg/path.hpp>
#include <std_srvs/srv/empty.hpp>

#include "Context/RobotContext.hpp"

namespace ROBOGait
{
namespace ros
{
namespace service
{
/**
 * @brief Client class to interact with robot services
 *
 * This class manages the execution of commands. It interacts with a ROS2 service to send command
 * requests and monitors the health of the nodes associated with the commands.
 */
class RobotServiceClient
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
   * @brief Enum to represent navigation action final result
   */
  enum class NavigationResult
  {
    UNKNOWN = 0,   /**< Unknown result */
    SUCCEEDED = 1, /**< Goal reached */
    CANCELED = 2,  /**< Goal canceled */
    ABORTED = 3    /**< Goal aborted */
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
   * @brief Get the singleton instance of the RobotServiceClient
   *
   * @return The singleton instance
   */
  static RobotServiceClient& getInstance();

  /** Delete copy constructor and assignment operator */
  RobotServiceClient(const RobotServiceClient&) = delete;
  RobotServiceClient& operator=(const RobotServiceClient&) = delete;

  /**
   * @brief Initialize the RobotServiceClient
   *
   * @param parent_node The parent ROS2 node
   *
   * @return true if initialization was successful, false otherwise
   */
  bool initialize(rclcpp::Node* parent_node);

  /**
   * @brief Check if the RobotServiceClient is initialized
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

  /** @brief Reinitialize the global localization
   *
   * @return true if the global localization was reinitialized successfully, false otherwise
   */
  bool reinitializeGlobalLocalization();

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

  /**
   * @brief Set callback invoked when a command service request completes
   */
  void setRequestCallback(const std::function<void(bool)>& callback);

  /**
   * @brief Request a path computation to a target pose
   *
   * @param x Goal x in map frame
   * @param y Goal y in map frame
   * @param theta Goal yaw in radians
   *
   * @return true if the request was sent, false otherwise
   */
  bool computePathToPose(double x, double y, double theta);

  /**
   * @brief Request a path computation through multiple poses
   *
   * @param points Goal poses in map frame as list of maps with keys x, y, and optional theta
   *
   * @return true if the request was sent, false otherwise
   */
  bool computePathThroughPoses(const QVariantList& points);

  /**
   * @brief Navigate to a target pose
   *
   * @param x Goal x in map frame
   * @param y Goal y in map frame
   * @param theta Goal yaw in radians
   *
   * @return true if the request was sent, false otherwise
   */
  bool navigateToPose(double x, double y, double theta);

  /**
   * @brief Navigate through multiple target poses
   *
   * @param points Goal poses in map frame as list of maps with keys x, y, and optional theta
   *
   * @return true if the request was sent, false otherwise
   */
  bool navigateThroughPoses(const QVariantList& points);

  /**
   * @brief Cancel the active navigate to pose action
   */
  bool cancelNavigateToPose();

  /**
   * @brief Set callback invoked when a compute path action finishes
   */
  void setPathResultCallback(const std::function<void(bool, const nav_msgs::msg::Path&)>& callback);

  /**
   * @brief Set callback invoked when a navigation action finishes
   */
  void setNavigationResultCallback(const std::function<void(NavigationResult)>& callback);

  /**
   * @brief Reset the client
   */
  void resetRobotServiceClient();

private:
  /**
   * @brief Enum to represent the type of a command request, used for routing the handling of command service responses
   */
  enum class CommandRequestType
  {
    StartMapping,    /**< Start mapping command */
    DeleteMap,       /**< Delete map command */
    StartNavigation, /**< Start navigation command */
    SaveMap,         /**< Save map command */
    StopCommand      /**< Stop command request, used for any command stop request and routes to same handling logic */
  };

  /**
   * @brief Struct to hold the context of a command request, used to route the handling of command service responses
   *
   * @param type The type of the command request
   * @param command_key The key of the command associated with the request (if applicable)
   * @param full_cmd The full command string that was sent in the request (if applicable)
   * @param map_name The name of the map associated with the request (if applicable)
   */
  struct CommandRequestContext
  {
    CommandRequestType type;
    std::string command_key;
    std::string full_cmd;
    std::string map_name;
  };

  /**
   * @brief Constructor of the RobotServiceClient class
   */
  RobotServiceClient();

  /**
   * @brief Destructor of the RobotServiceClient class
   */
  ~RobotServiceClient() = default;

  /**
   * @brief Load commands from the YAML configuration file
   *
   * @return true if commands were loaded successfully, false otherwise
   */
  bool loadCommands();

  /**
   * @brief Load timeout configuration from YAML configuration file
   *
   * @return true if timeouts were loaded successfully, false otherwise
   */
  bool loadTimeouts();

  /**
   * @brief Call a command service
   *
   * @param cmd The command to call
   * @param execute Whether to execute the command
   * @param context Metadata to route completion handling
   *
   * @return true if the service call was successful, false otherwise
   */
  bool callCommandServiceAsync(const std::string& cmd, bool execute, const CommandRequestContext& context);

  /**
   * @brief Handle the response from a command service request
   *
   * @param context The context of the command request, used to determine how to handle the response
   * @param success Whether the command service request was successful
   */
  void handleCommandResponse(const CommandRequestContext& context, bool success);

  /**
   * @brief Handle the result of a start mapping command
   *
   * @param success Whether the command was successful
   * @param full_cmd The full command string
   */
  void handleStartMappingResult(bool success, const std::string& full_cmd);

  /**
   * @brief Handle the result of a delete map command
   *
   * @param success Whether the command was successful
   * @param map_name The name of the map that was attempted to be deleted
   */
  void handleDeleteMapResult(bool success, const std::string& map_name);

  /**
   * @brief Handle the result of a start navigation command
   *
   * @param success Whether the command was successful
   * @param full_cmd The full command string
   * @param map_name The name of the map that was attempted to be used for navigation
   */
  void handleStartNavigationResult(bool success, const std::string& full_cmd, const std::string& map_name);

  /**
   * @brief Handle the result of a save map command
   *
   * @param success Whether the command was successful
   * @param full_cmd The full command string
   */
  void handleSaveMapResult(bool success, const std::string& full_cmd);

  /**
   * @brief Handle the result of a stop command
   *
   * @param success Whether the command was successful
   * @param key The key of the command that was attempted to be stopped
   * @param full_cmd The full command string
   */
  void handleStopCommandResult(bool success, const std::string& key, const std::string& full_cmd);

  /**
   * @brief Handle the result of a global localization command
   *
   * @param success Whether the command was successful
   */
  void handleGlobalLocalizationResult(bool success);

  /**
   * @brief Notify the result of a command service request through the callback
   *
   * @param success Whether the command service request was successful
   */
  void notifyRequestResult(bool success);

  /**
   * @brief Notify the result of a navigation action through the callback
   */
  void notifyNavigationResult(NavigationResult result);

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

  /**
   * @brief Callback for compute path action results
   */
  void resultComputePathToPoseCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathToPose>::WrappedResult& result);

  /**
   * @brief Callback for compute path through poses action results
   */
  void resultComputePathThroughPosesCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathThroughPoses>::WrappedResult& result);

  /**
   * @brief Callback for navigate to pose goal response
   */
  void goalResponseNavigateToPoseCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr& goal_handle);

  /**
   * @brief Callback for navigate to pose cancel response
   */
  void cancelNavigateToPoseCallback(typename rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::CancelResponse::SharedPtr response);

  /**
   * @brief Callback for navigate to pose action results
   */
  void resultNavigateToPoseCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult& result);

  /**
   * @brief Callback for navigate through poses action results
   */
  void resultNavigateThroughPosesCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateThroughPoses>::WrappedResult& result);

  rclcpp::Node* parent_node_;                                                          /**< The parent ROS2 node */
  rclcpp::Client<command_executor_msgs::srv::Cmd>::SharedPtr cli_cmd_;                 /**< The command service client */
  rclcpp::Client<command_executor_msgs::srv::GetMapData>::SharedPtr cli_get_map_data_; /**< The get map data service client */
  rclcpp::Client<std_srvs::srv::Empty>::SharedPtr cli_global_localization_;            /**< Global localization service client */
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr pub_map_data_;            /**< Publisher for map data */
  rclcpp_action::Client<nav2_msgs::action::ComputePathThroughPoses>::SharedPtr
      ac_compute_path_through_poses_;                                                                   /**< Action client for compute path through poses */
  rclcpp_action::Client<nav2_msgs::action::ComputePathToPose>::SharedPtr ac_compute_path_to_pose_;      /**< Action client for compute path to pose */
  rclcpp_action::Client<nav2_msgs::action::NavigateThroughPoses>::SharedPtr ac_navigate_through_poses_; /**< Action client for navigate through poses */
  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr ac_navigate_to_pose_;             /**< Action client for navigate to pose */
  rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr nav_goal_handle_;       /**< Active navigate to pose goal handle */
  rclcpp::CallbackGroup::SharedPtr cb_group_;                                                           /**< The callback group for the command executor */
  rclcpp::TimerBase::SharedPtr timer_health_;                                                           /**< The health timer */

  std::function<void(bool)> request_callback_;                                 /**< Callback to notify the result of command service requests */
  std::function<void(bool, const nav_msgs::msg::Path&)> path_result_callback_; /**< Callback for compute path results */
  std::function<void(NavigationResult)> navigation_result_callback_;           /**< Callback for navigation action results */

  std::optional<ROBOGait::context::RobotContext> context_; /**< The robot context */

  std::unordered_map<std::string, CommandInfo> commands_; /**< Map of command names to their info. Keys are command names, values are CommandInfo structs. */
  std::unordered_map<std::string, CommandState>
      command_states_; /**< Map of command names to their states. Keys are command names, values are CommandState structs. */

  bool initialized_;              /**< Flag indicating if the client is initialized */
  bool pending_stop_after_save_;  /**< Flag indicating if a stop is pending after save */
  bool map_saver_stop_requested_; /**< Flag indicating if a stop is requested for the map saver */
  bool nav_goal_active_;          /**< Flag indicating if there is an active navigation goal */
  bool cancel_requested_;         /**< Flag indicating if a cancel has been requested for the active navigation goal */
  bool cancel_in_progress_;       /**< Flag indicating if a cancel is in progress for the active navigation goal */

  std::chrono::seconds start_stop_timeout_s_; /**< Configurable timeout for STARTING/STOPPING transitions */

  static constexpr const char* KEY_CARTOGRAPHER = "cartographer"; /**< Key for the cartographer command */
  static constexpr const char* KEY_MAP_SAVER = "map_saver";       /**< Key for the map saver command */
  static constexpr const char* KEY_DELETE_MAP = "delete_map";     /**< Key for the delete map command */
  static constexpr const char* KEY_NAVIGATION = "navigation";     /**< Key for the navigation command */

  static constexpr std::chrono::milliseconds HEALTH_CHECK_PERIOD = std::chrono::milliseconds(100);                 /**< Health check period */
  static constexpr std::chrono::seconds START_STOP_TIMEOUT = std::chrono::seconds(25);                             /**< Start/stop timeout */
  static constexpr std::chrono::milliseconds MAP_SAVER_FORCE_STOP_DELAY = std::chrono::milliseconds(2000);         /**< Map saver force stop delay */
  static constexpr std::chrono::milliseconds MAP_SAVER_STARTUP_ASSUME_STOP_DELAY = std::chrono::milliseconds(500); /**< Map saver startup assume stop delay */
};
} // namespace service
} // namespace ros
} // namespace ROBOGait
