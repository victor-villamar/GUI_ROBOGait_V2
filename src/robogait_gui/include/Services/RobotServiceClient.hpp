#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <QVariantList>

#include <rclcpp/callback_group.hpp>
#include <rclcpp/client.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/timer.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include <command_executor_msgs/srv/cmd.hpp>
#include <command_executor_msgs/srv/get_map_data.hpp>
#include <lifecycle_msgs/srv/get_state.hpp>
#include <nav2_msgs/action/compute_path_to_pose.hpp>
#include <nav2_msgs/action/follow_path.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <nav2_msgs/msg/speed_limit.hpp>
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
   * @brief Enum to identify the asynchronous service operation that completed
   */
  enum class RequestType
  {
    StartMapping,      /**< Start mapping command request */
    DeleteMap,         /**< Delete map command request */
    StartNavigation,   /**< Start navigation command request */
    SaveMap,           /**< Save map command request */
    StopCommand,       /**< Stop command request */
    GetMapData,        /**< Get map data request */
    AmclReadiness,     /**< Wait for the AMCL lifecycle node to become active */
    GlobalLocalization /**< Global localization request */
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
   * @return true if the asynchronous start request was queued, false otherwise
   */
  bool startMapping();

  /**
   * @brief Stop the mapping process
   *
   * @param save_map Whether to save the map
   * @param map_name The name of the map to save
   *
   * @return true if the asynchronous stop request was queued, false otherwise
   */
  bool stopMapping(bool save_map, const std::string& map_name);

  /**
   * @brief Delete a map
   *
   * @param map_name The name of the map to delete
   *
   * @return true if the asynchronous delete request was queued, false otherwise
   */
  bool deleteMap(const std::string& map_name);

  /**
   * @brief Request the map data for a given map name
   *
   * @param map_name The name of the map to request data for
   *
   * @return true if the asynchronous map data request was queued, false otherwise
   */
  bool requestMapData(const std::string& map_name);

  /**
   * @brief Start the navigation process
   *
   * @param map_name The name of the map to use for navigation
   *
   * @return true if the asynchronous start request was queued, false otherwise
   */
  bool startNavigation(const std::string& map_name);

  /**
   * @brief Stop the navigation process
   *
   * @return true if the asynchronous stop request was queued, false otherwise
   */
  bool stopNavigation();

  /**
   * @brief Create the publisher used to limit Nav2 controller speed during a test
   *
   * @return true if the publisher is available, false otherwise
   */
  bool prepareNavigationSpeedLimitPublisher();

  /**
   * @brief Publish an absolute Nav2 linear speed limit
   *
   * The requested value is clamped to the maximum linear velocity accepted by
   * RobotSettings so navigation and manual control share the same safety limit.
   *
   * @param speed_limit Requested maximum linear velocity in m/s
   *
   * @return true if the limit was published, false otherwise
   */
  bool publishNavigationSpeedLimit(double speed_limit);

  /**
   * @brief Restore the controller's configured base speed by publishing zero
   *
   * @return true if the reset was published or no publisher exists, false otherwise
   */
  bool resetNavigationSpeedLimit();

  /**
   * @brief Destroy the Nav2 speed limit publisher
   */
  void destroyNavigationSpeedLimitPublisher();

  /**
   * @brief Check whether a non-zero navigation speed limit has been published
   */
  bool isNavigationSpeedLimitApplied() const;

  /** @brief Reinitialize the global localization
   *
   * @return true if the asynchronous localization request was queued, false otherwise
   */
  bool reinitializeGlobalLocalization();

  /**
   * @brief Wait asynchronously until the AMCL lifecycle node is active
   *
   * @return true if the readiness operation was queued, false otherwise
   */
  bool waitForAmclActive();

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
   * @brief Set callback invoked when an asynchronous service request completes
   *
   * @param callback Callback receiving the request type and its terminal result
   */
  void setRequestCallback(const std::function<void(RequestType, bool)>& callback);

  /**
   * @brief Request a path computation to a target pose
   *
   * @param x Goal x in map frame
   * @param y Goal y in map frame
   * @param theta Goal yaw in radians
   *
   * @return true if the asynchronous action request was queued, false otherwise
   */
  bool computePathToPose(double x, double y, double theta);

  /**
   * @brief Navigate to a target pose
   *
   * @param x Goal x in map frame
   * @param y Goal y in map frame
   * @param theta Goal yaw in radians
   *
   * @return true if the asynchronous action request was queued, false otherwise
   */
  bool navigateToPose(double x, double y, double theta);

  /**
   * @brief Store the edited manual spline as a path for FollowPath execution
   */
  bool setManualFollowPath(const QVariantList& points);

  /**
   * @brief Follow the last prepared path from the manual path flow
   *
   * @return true if the asynchronous action request was queued, false otherwise
   */
  bool followLastComputedPath();

  /**
   * @brief Cancel active navigation actions
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
   * @brief Enum to represent the type of a command request, used for routing command service responses
   */
  enum class CommandRequestType
  {
    StartMapping,    /**< Start mapping command */
    DeleteMap,       /**< Delete map command */
    StartNavigation, /**< Start navigation command */
    SaveMap,         /**< Save map command */
    StopCommand      /**< Stop command request */
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
   * @brief Enum to represent the type of an asynchronous ROS operation
   */
  enum class OperationType
  {
    StartMapping,       /**< Start mapping command service operation */
    DeleteMap,          /**< Delete map command service operation */
    StartNavigation,    /**< Start navigation command service operation */
    SaveMap,            /**< Save map command service operation */
    StopCommand,        /**< Stop command service operation */
    GetMapData,         /**< Get map data service operation */
    AmclReadiness,      /**< AMCL lifecycle readiness operation */
    GlobalLocalization, /**< Global localization service operation */
    ComputePathToPose,  /**< Compute path to pose action operation */
    NavigateToPose,     /**< Navigate to pose action operation */
    FollowPath          /**< Follow path action operation */
  };

  /**
   * @brief Enum to represent the current phase of an asynchronous ROS operation
   */
  enum class OperationPhase
  {
    WaitingEndpoint,        /**< Waiting for a service or action server to become available */
    WaitingServiceResponse, /**< Waiting for a response to a service request */
    WaitingGoalResponse,    /**< Waiting for an action server to accept or reject a goal */
    WaitingLifecycleActive  /**< Waiting for a lifecycle node to reach the active state */
  };

  /**
   * @brief Enum to represent why a pending operation ended before normal completion
   */
  enum class OperationStopReason
  {
    Timeout,           /**< The configured timeout expired */
    ContextInvalidated /**< The selected robot context changed or was cleared */
  };

  /** @brief Unique identifier assigned to a pending operation */
  using OperationId = std::uint64_t;

  /** @brief Callback invoked when a pending operation times out or its context is invalidated */
  using OperationStopCallback = std::function<void(OperationStopReason)>;

  /** @brief Callback invoked periodically while an operation waits for a ROS endpoint */
  using OperationPollCallback = std::function<void()>;

  /**
   * @brief Struct to hold the state of a pending asynchronous ROS operation
   *
   * @param id The unique identifier of the operation
   * @param type The type of the operation
   * @param phase The current phase of the operation
   * @param deadline The steady-clock time point at which the current phase expires
   * @param context_generation The robot context generation in which the operation was created
   */
  struct PendingOperation
  {
    OperationId id;
    OperationType type;
    OperationPhase phase;
    std::chrono::steady_clock::time_point deadline;
    std::uint64_t context_generation;
  };

  /**
   * @brief Struct to hold a pending operation and its cleanup callback
   *
   * @param operation The metadata and current state of the pending operation
   * @param poll_callback The callback used to check endpoint readiness without blocking
   * @param stop_callback The cleanup callback invoked on timeout or context invalidation
   */
  struct PendingOperationEntry
  {
    PendingOperation operation;
    OperationPollCallback poll_callback;
    OperationStopCallback stop_callback;
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
   * @brief Register a new asynchronous operation using the configured timeout for its phase
   *
   * @param type Operation type
   * @param phase Initial operation phase
   * @param stop_callback Cleanup callback invoked on timeout or context invalidation
   *
   * @return Metadata token that must be captured by asynchronous callbacks
   */
  PendingOperation registerPendingOperation(OperationType type, OperationPhase phase, OperationStopCallback stop_callback = {});

  /**
   * @brief Move a pending operation to another phase and restart its phase timeout
   *
   * @param id Operation identifier
   * @param context_generation Robot context generation captured by the operation
   * @param phase New operation phase
   * @param stop_callback Cleanup callback invoked on timeout or context invalidation
   * @param deadline Optional absolute deadline preserved across repeated readiness requests
   *
   * @return true when the operation is still current and was updated, false for a stale callback
   */
  bool transitionPendingOperation(OperationId id, std::uint64_t context_generation, OperationPhase phase, OperationStopCallback stop_callback = {},
                                  std::optional<std::chrono::steady_clock::time_point> deadline = std::nullopt);

  /**
   * @brief Set the non-blocking readiness callback for a pending operation
   *
   * @return true when the operation is still current and was updated, false for a stale callback
   */
  bool setPendingOperationPollCallback(OperationId id, std::uint64_t context_generation, OperationPollCallback poll_callback);

  /**
   * @brief Check whether an asynchronous callback belongs to the current robot context
   */
  bool isContextGenerationCurrent(std::uint64_t context_generation);

  /**
   * @brief Finish and remove a pending operation
   *
   * @return Operation metadata, or std::nullopt when the callback is stale
   */
  std::optional<PendingOperation> completePendingOperation(OperationId id, std::uint64_t context_generation);

  /**
   * @brief Invalidate all operations belonging to the current robot context
   */
  void invalidatePendingOperations();

  /**
   * @brief Remove expired operations and run their cleanup callbacks
   */
  void onPendingOperationsTimer();

  /**
   * @brief Get the configured timeout for an operation phase
   */
  std::chrono::milliseconds timeoutForPhase(OperationPhase phase) const;

  /**
   * @brief Read a navigation plugin id from configuration
   *
   * @param key YAML key
   * @param default_value Default id used when the key is missing or invalid
   * @return Configured navigation plugin id
   */
  std::string getConfiguredNavigationId(std::string_view key, std::string_view default_value) const;

  /**
   * @brief Check whether navigation is configured to use simulation time
   *
   * @return true if navigation uses simulation time, false otherwise
   */
  bool useNavigationSimTime() const;

  /**
   * @brief Call a command service
   *
   * @param cmd The command to call
   * @param execute Whether to execute the command
   * @param context Metadata to route completion handling
   *
   * @return true if the asynchronous service operation was queued, false otherwise
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
   * @brief Notify the terminal result of an asynchronous service operation
   *
   * @param type The type of request that completed
   * @param success Whether the request was successful
   */
  void notifyRequestResult(RequestType type, bool success);

  /**
   * @brief Query AMCL lifecycle state without blocking
   *
   * @param operation_id Identifier of the AMCL readiness operation
   * @param context_generation Robot context generation captured by the operation
   * @param activation_deadline Absolute deadline for AMCL to become active
   */
  void pollAmclState(OperationId operation_id, std::uint64_t context_generation, std::chrono::steady_clock::time_point activation_deadline);

  /**
   * @brief Notify the result of a navigation action through the callback
   */
  void notifyNavigationResult(NavigationResult result);

  /**
   * @brief Call the get map data service asynchronously and publish the resulting map
   *
   * @param map_name The name of the map
   *
   * @return true if the operation was queued, false otherwise
   */
  bool callGetMapDataServiceAsync(const std::string& map_name);

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
   * @brief Get the selected robot namespace without its leading slash
   *
   * @return The normalized namespace, or an empty string if no namespace is configured
   */
  std::string getRobotNamespace() const;

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
   * @return true if the asynchronous save request was queued, false otherwise
   */
  bool saveMap(const std::string& map_name);

  /**
   * @brief Stop a command
   *
   * @param key The key of the command to stop
   *
   * @return true if the asynchronous stop request was queued, false otherwise
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
   * @brief Callback for follow path goal response
   */
  void goalResponseFollowPathCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::FollowPath>::SharedPtr& goal_handle);

  /**
   * @brief Callback for follow path cancel response
   */
  void cancelFollowPathCallback(typename rclcpp_action::Client<nav2_msgs::action::FollowPath>::CancelResponse::SharedPtr response);

  /**
   * @brief Callback for follow path action results
   */
  void resultFollowPathCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::FollowPath>::WrappedResult& result);

  rclcpp::Node* parent_node_;                                                           /**< The parent ROS2 node */
  rclcpp::Client<command_executor_msgs::srv::Cmd>::SharedPtr cli_cmd_;                  /**< The command service client */
  rclcpp::Client<command_executor_msgs::srv::GetMapData>::SharedPtr cli_get_map_data_;  /**< The get map data service client */
  rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedPtr cli_amcl_get_state_;         /**< The AMCL lifecycle state service client */
  rclcpp::Client<std_srvs::srv::Empty>::SharedPtr cli_global_localization_;             /**< Global localization service client */
  rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr pub_map_data_;             /**< Publisher for map data */
  rclcpp::Publisher<nav2_msgs::msg::SpeedLimit>::SharedPtr pub_navigation_speed_limit_; /**< Publisher for the Nav2 linear speed limit */

  rclcpp_action::Client<nav2_msgs::action::ComputePathToPose>::SharedPtr ac_compute_path_to_pose_; /**< Action client for compute path to pose */
  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr ac_navigate_to_pose_;        /**< Action client for navigate to pose */
  rclcpp_action::Client<nav2_msgs::action::FollowPath>::SharedPtr ac_follow_path_;                 /**< Action client for follow path */

  rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr nav_goal_handle_;     /**< Active navigate to pose goal handle */
  rclcpp_action::ClientGoalHandle<nav2_msgs::action::FollowPath>::SharedPtr follow_path_goal_handle_; /**< Active follow path goal handle */
  rclcpp::CallbackGroup::SharedPtr cb_group_;                                                         /**< The callback group for the command executor */
  rclcpp::TimerBase::SharedPtr timer_health_;                                                         /**< The health timer */
  rclcpp::TimerBase::SharedPtr timer_pending_operations_;                                             /**< Timer that expires asynchronous ROS operations */

  std::function<void(RequestType, bool)> request_callback_;                    /**< Callback to notify typed service request results */
  std::function<void(bool, const nav_msgs::msg::Path&)> path_result_callback_; /**< Callback for compute path results */
  std::function<void(NavigationResult)> navigation_result_callback_;           /**< Callback for navigation action results */

  std::optional<ROBOGait::context::RobotContext> context_; /**< The robot context */

  std::unordered_map<std::string, CommandInfo> commands_; /**< Map of command names to their info. Keys are command names, values are CommandInfo structs. */
  std::unordered_map<std::string, CommandState>
      command_states_; /**< Map of command names to their states. Keys are command names, values are CommandState structs. */

  std::optional<nav_msgs::msg::Path> manual_follow_path_; /**< The last prepared manual path, if any */

  bool initialized_;                    /**< Flag indicating if the client is initialized */
  bool nav_goal_active_;                /**< Flag indicating if there is an active navigation goal */
  bool cancel_in_progress_;             /**< Flag indicating if a cancel is in progress for the active navigation goal */
  bool navigation_speed_limit_applied_; /**< Flag indicating if a non-zero Nav2 speed limit is active */

  std::chrono::seconds start_stop_timeout_s_;               /**< Configurable timeout for STARTING/STOPPING transitions */
  std::chrono::milliseconds endpoint_availability_timeout_; /**< Maximum wait for a ROS service or action server to become available */
  std::chrono::milliseconds service_response_timeout_;      /**< Maximum wait for a service response after sending its request */
  std::chrono::milliseconds action_goal_response_timeout_;  /**< Maximum wait for an action server to accept or reject a goal */
  std::chrono::milliseconds readiness_poll_period_;         /**< Interval between non-blocking ROS endpoint readiness checks */

  std::mutex pending_operations_mutex_;                                       /**< Protects pending operation state shared by Qt and ROS threads */
  std::unordered_map<OperationId, PendingOperationEntry> pending_operations_; /**< Operations waiting for an endpoint or asynchronous response */
  OperationId next_operation_id_;                                             /**< Monotonic identifier assigned to the next operation */
  std::uint64_t context_generation_;                                          /**< Incremented whenever the selected robot context changes */

  static constexpr const char* KEY_CARTOGRAPHER = "cartographer"; /**< Key for the cartographer command */
  static constexpr const char* KEY_MAP_SAVER = "map_saver";       /**< Key for the map saver command */
  static constexpr const char* KEY_DELETE_MAP = "delete_map";     /**< Key for the delete map command */
  static constexpr const char* KEY_NAVIGATION = "navigation";     /**< Key for the navigation command */

  static constexpr std::string_view CONFIG_MANUAL_FOLLOW_PATH_CONTROLLER_ID =
      "navigation.manual_follow_path.controller_id"; /**< Config key for direct FollowPath manual-path controller id */
  static constexpr std::string_view CONFIG_MANUAL_FOLLOW_PATH_GOAL_CHECKER_ID =
      "navigation.manual_follow_path.goal_checker_id"; /**< Config key for direct FollowPath manual-path goal checker id */
  static constexpr std::string_view CONFIG_NAVIGATION_USE_SIM_TIME = "navigation.use_sim_time"; /**< Config key for Nav2 simulation time */

  static constexpr std::string_view DEFAULT_MANUAL_FOLLOW_PATH_CONTROLLER_ID = "FollowPath";      /**< Default controller used for manual spline paths */
  static constexpr std::string_view DEFAULT_FOLLOW_PATH_GOAL_CHECKER_ID = "general_goal_checker"; /**< Default FollowPath goal checker id */

  static constexpr std::chrono::milliseconds HEALTH_CHECK_PERIOD = std::chrono::milliseconds(100);            /**< Health check period */
  static constexpr std::chrono::seconds START_STOP_TIMEOUT = std::chrono::seconds(25);                        /**< Start/stop timeout */
  static constexpr std::chrono::milliseconds ENDPOINT_AVAILABILITY_TIMEOUT = std::chrono::milliseconds(5000); /**< Endpoint availability timeout */
  static constexpr std::chrono::milliseconds SERVICE_RESPONSE_TIMEOUT = std::chrono::milliseconds(5000);      /**< Service response timeout */
  static constexpr std::chrono::milliseconds ACTION_GOAL_RESPONSE_TIMEOUT = std::chrono::milliseconds(5000);  /**< Action goal response timeout */
  static constexpr std::chrono::milliseconds READINESS_POLL_PERIOD = std::chrono::milliseconds(100);          /**< Endpoint readiness poll period */
};
} // namespace service
} // namespace ros
} // namespace ROBOGait
