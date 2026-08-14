#include <algorithm>
#include <filesystem>
#include <optional>
#include <vector>

#include <action_msgs/srv/cancel_goal.hpp>
#include <yaml-cpp/yaml.h>

#include <QDebug>
#include <QDir>

#include "Loader/BootStrapLoader.hpp"
#include "Loader/MapFileLoader.hpp"
#include "Loader/YamlLoader.hpp"
#include "Map/Interaction/Geometry/StrokeProcessor.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/Define.hpp"
#include "Ros/QoSProfiles.hpp"
#include "Ros/TopicsName.hpp"
#include "Services/RobotServiceClient.hpp"

using namespace ROBOGait::ros::service;

RobotServiceClient& RobotServiceClient::getInstance()
{
  static RobotServiceClient instance;
  return instance;
}

RobotServiceClient::RobotServiceClient() :
    parent_node_(nullptr),
    timer_health_(nullptr),
    context_(std::nullopt),
    manual_follow_path_(std::nullopt),
    initialized_(false),
    pending_stop_after_save_(false),
    map_saver_stop_requested_(false),
    nav_goal_active_(false),
    cancel_in_progress_(false),
    start_stop_timeout_s_(START_STOP_TIMEOUT)
{
}

bool RobotServiceClient::initialize(rclcpp::Node* parent_node)
{
  if (!parent_node)
  {
    qCritical() << "[RobotServiceClient::initialize] Invalid parent_node";
    return false;
  }

  if (initialized_)
  {
    if (parent_node_ == parent_node && cb_group_)
    {
      qInfo() << "[RobotServiceClient::initialize] Already initialized with the same parent node, skipping reinitialization";
      return true;
    }

    resetRobotServiceClient();
  }

  parent_node_ = parent_node;

  if (!loadCommands())
  {
    qCritical() << "[RobotServiceClient::initialize] Failed to load commands";
    return false;
  }

  if (!loadTimeouts())
  {
    qCritical() << "[RobotServiceClient::initialize] Failed to load timeouts";
    return false;
  }

  cb_group_ = parent_node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  if (!cb_group_)
  {
    qCritical() << "[RobotServiceClient::initialize] Failed to create callback group";
    return false;
  }

  initialized_ = true;
  return initialized_;
}

bool RobotServiceClient::isInitialized() const { return initialized_; }

void RobotServiceClient::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;

  if (initialized_)
  {
    if (!rebuildClient())
    {
      qCritical() << "[RobotServiceClient::setRobotContext] Failed to rebuild command client";
    }
  }
}

void RobotServiceClient::clearRobotContext()
{
  if (context_)
  {
    context_->clear();
  }
  cli_cmd_.reset();
  cli_get_map_data_.reset();
  cli_global_localization_.reset();
  ac_compute_path_to_pose_.reset();
  ac_navigate_to_pose_.reset();
  ac_follow_path_.reset();
  nav_goal_active_ = false;
  nav_goal_handle_.reset();
  follow_path_goal_handle_.reset();
  manual_follow_path_.reset();
  pub_map_data_.reset();
  command_states_.clear();
  pending_stop_after_save_ = false;
  map_saver_stop_requested_ = false;
  stopHealthTimer();
}

bool RobotServiceClient::startMapping()
{
  if (!validateCommandKey(KEY_CARTOGRAPHER))
  {
    qCritical() << "[RobotServiceClient::startMapping] Command key" << KEY_CARTOGRAPHER << "is not valid";
    return false;
  }

  const RobotServiceClient::CommandInfo& cmd_info = commands_[KEY_CARTOGRAPHER];

  const std::string full_cmd = buildCommand(cmd_info.cmd, cmd_info.append_args);

  pending_stop_after_save_ = false;
  map_saver_stop_requested_ = false;

  CommandRequestContext context{CommandRequestType::StartMapping, std::string(KEY_CARTOGRAPHER), full_cmd, std::string()};

  const bool sent = callCommandServiceAsync(full_cmd, true, context);

  if (!sent)
  {
    qCritical() << "[RobotServiceClient::startMapping] Failed to initialize mapping";
    return false;
  }

  return true;
}

bool RobotServiceClient::stopMapping(bool save_map, const std::string& map_name)
{
  if (save_map)
  {
    if (!saveMap(map_name))
    {
      qCritical() << "[RobotServiceClient::stopMapping] Failed to start map saving";
      return false;
    }

    pending_stop_after_save_ = true;
    map_saver_stop_requested_ = false;
    startHealthTimer();
    return true;
  }

  pending_stop_after_save_ = false;
  return stopCommand(KEY_CARTOGRAPHER);
}

bool RobotServiceClient::deleteMap(const std::string& map_name)
{
  if (map_name.empty())
  {
    qCritical() << "[RobotServiceClient::deleteMap] Map name is empty";
    return false;
  }

  const std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);
  if (safe_name.empty())
  {
    qCritical() << "[RobotServiceClient::deleteMap] Sanitized map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[RobotServiceClient::deleteMap] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[RobotServiceClient::deleteMap] Map path is empty in YAML configuration";
    return false;
  }

  if (!validateCommandKey(KEY_DELETE_MAP))
  {
    qCritical() << "[RobotServiceClient::deleteMap] Command key" << KEY_DELETE_MAP << "is not valid";
    return false;
  }

  const RobotServiceClient::CommandInfo& cmd_info = commands_[KEY_DELETE_MAP];

  std::string args = cmd_info.append_args;

  const std::unordered_map<std::string, std::string> vars = {
      {"{map_path}", map_path},
      {"{map_name}", safe_name},
  };

  args = replacePlaceholders(args, vars);

  const std::string full_cmd = buildCommand(cmd_info.cmd, args);

  CommandRequestContext context{CommandRequestType::DeleteMap, std::string(KEY_DELETE_MAP), full_cmd, map_name};

  const bool sent = callCommandServiceAsync(full_cmd, true, context);

  if (!sent)
  {
    qCritical() << "[RobotServiceClient::deleteMap] Failed to delete map";
    return false;
  }

  return true;
}

bool RobotServiceClient::requestMapData(const std::string& map_name)
{
  if (map_name.empty())
  {
    qCritical() << "[RobotServiceClient::requestMapData] Map name is empty";
    return false;
  }

  std::optional<std::string> yaml_info;
  std::optional<std::vector<uint8_t>> pgm_info;

  if (!callGetMapDataService(map_name, yaml_info, pgm_info))
  {
    qCritical() << "[RobotServiceClient::requestMapData] Failed to call get map data service for map " << QString::fromStdString(map_name);
    return false;
  }

  ROBOGait::loader::MapFileLoader map_loader;

  const auto map_result = map_loader.loadMap(*yaml_info, *pgm_info);

  if (!map_result)
  {
    qCritical().noquote() << QString("[RobotServiceClient::requestMapData] Failed to load map data for map %1\n%2")
                                 .arg(QString::fromStdString(map_name), QString::fromStdString(map_result.error));
    return false;
  }

  auto occupancy_grid = map_result.occupancy_grid.value();
  occupancy_grid.header.frame_id = context_ ? context_->resolveFrame(ROBOGait::ros::topics::TF_MAP_FRAME) : ROBOGait::ros::topics::TF_MAP_FRAME;

  return publishMapDataOnce(occupancy_grid);
}

bool RobotServiceClient::startNavigation(const std::string& map_name)
{
  if (map_name.empty())
  {
    qCritical() << "[RobotServiceClient::startNavigation] Map name is empty";
    return false;
  }

  std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);
  if (safe_name.empty())
  {
    qCritical() << "[RobotServiceClient::startNavigation] Sanitized map name is empty";
    return false;
  }

  safe_name = safe_name + ".yaml";

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[RobotServiceClient::startNavigation] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[RobotServiceClient::startNavigation] Map path is empty in YAML configuration";
    return false;
  }

  const bool navigation_use_sim_time = useNavigationSimTime();
  const std::string navigation_use_sim_time_arg = navigation_use_sim_time ? "true" : "false";
  const std::string nav2_params_path = yaml_loader.getValue<std::string>("navigation.nav2_params_path", "/.local/robogait/params/");
  const std::string nav2_params_file = yaml_loader.getValue<std::string>("navigation.nav2_params_file", "nav2_params.yaml");

  if (nav2_params_path.empty())
  {
    qCritical() << "[RobotServiceClient::startNavigation] Nav2 params path is empty in YAML configuration";
    return false;
  }
  if (nav2_params_file.empty())
  {
    qCritical() << "[RobotServiceClient::startNavigation] Nav2 params file is empty in YAML configuration";
    return false;
  }

  if (!validateCommandKey(KEY_NAVIGATION))
  {
    qCritical() << "[RobotServiceClient::startNavigation] Command key" << KEY_NAVIGATION << "is not valid";
    return false;
  }

  const RobotServiceClient::CommandInfo& cmd_info = commands_[KEY_NAVIGATION];
  std::string args = cmd_info.append_args;

  const std::unordered_map<std::string, std::string> vars = {{"{map_path}", map_path},
                                                             {"{map_name}", safe_name},
                                                             {"{navigation_use_sim_time}", navigation_use_sim_time_arg},
                                                             {"{nav2_params_path}", nav2_params_path},
                                                             {"{nav2_params_file}", nav2_params_file}};

  args = replacePlaceholders(args, vars);

  const std::string full_cmd = buildCommand(cmd_info.cmd, args);

  CommandRequestContext context{CommandRequestType::StartNavigation, std::string(KEY_NAVIGATION), full_cmd, map_name};

  const bool sent = callCommandServiceAsync(full_cmd, true, context);

  if (!sent)
  {
    qCritical() << "[RobotServiceClient::startNavigation] Failed to start navigation";
    return false;
  }

  return true;
}

bool RobotServiceClient::stopNavigation()
{
  if (getCommandStatus(KEY_NAVIGATION) == RobotServiceClient::CommandStatus::IDLE)
  {
    return true;
  }

  return stopCommand(KEY_NAVIGATION);
}

bool RobotServiceClient::reinitializeGlobalLocalization()
{
  if (!initialized_)
  {
    qCritical() << "[RobotServiceClient::reinitializeGlobalLocalization] RobotServiceClient is not initialized";
    notifyRequestResult(false);
    return false;
  }

  if (!cli_global_localization_)
  {
    qCritical() << "[RobotServiceClient::reinitializeGlobalLocalization] Global localization client is not available";
    notifyRequestResult(false);
    return false;
  }

  if (!cli_global_localization_->wait_for_service(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[RobotServiceClient::reinitializeGlobalLocalization] Service is not available after waiting";
    notifyRequestResult(false);
    return false;
  }

  auto request = std::make_shared<std_srvs::srv::Empty::Request>();

  cli_global_localization_->async_send_request(request,
                                               [this](rclcpp::Client<std_srvs::srv::Empty>::SharedFuture future)
                                               {
                                                 bool ok = false;

                                                 auto response = future.get();
                                                 ok = static_cast<bool>(response);

                                                 handleGlobalLocalizationResult(ok);
                                               });

  return true;
}

bool RobotServiceClient::isNodeAlive(const std::string& node_name) const
{
  if (!parent_node_)
  {
    qCritical() << "[RobotServiceClient::isNodeAlive] Parent node is not set";
    return false;
  }

  if (node_name.empty())
  {
    qCritical() << "[RobotServiceClient::isNodeAlive] Node name is empty";
    return false;
  }

  auto graph = parent_node_->get_node_graph_interface();
  if (!graph)
  {
    qCritical() << "[RobotServiceClient::isNodeAlive] Failed to get node graph interface";
    return false;
  }

  std::string expected_name = node_name;
  if (!expected_name.empty() && expected_name.front() == '/')
  {
    expected_name.erase(0, 1);
  }

  bool require_namespace = false;
  std::string target_ns = "/";
  if (context_ && context_->usesNamespace())
  {
    const QString ns = context_->topicNamespace();
    if (!ns.isEmpty())
    {
      require_namespace = true;
      target_ns = ns.toStdString();
    }
  }

  const auto nodes_with_ns = graph->get_node_names_and_namespaces();

  for (const auto& [name, ns] : nodes_with_ns)
  {
    if (name != expected_name)
    {
      continue;
    }

    if (!require_namespace)
    {
      return true;
    }

    if (ns == target_ns || ns.empty() || ns == "/")
    {
      return true;
    }
  }

  return false;
}

RobotServiceClient::CommandStatus RobotServiceClient::getActiveCommandStatus() const
{
  const auto nav_status = getCommandStatus(KEY_NAVIGATION);
  if (isCommandActive(nav_status))
  {
    return nav_status;
  }

  const auto cart_status = getCommandStatus(KEY_CARTOGRAPHER);
  if (isCommandActive(cart_status))
  {
    return cart_status;
  }

  if (cart_status != RobotServiceClient::CommandStatus::IDLE)
  {
    return cart_status;
  }

  if (nav_status == RobotServiceClient::CommandStatus::ERROR)
  {
    return nav_status;
  }

  return nav_status;
}

std::string RobotServiceClient::getActiveCommandKey() const
{
  const auto nav_status = getCommandStatus(KEY_NAVIGATION);
  if (isCommandActive(nav_status))
  {
    return std::string(KEY_NAVIGATION);
  }

  const auto cart_status = getCommandStatus(KEY_CARTOGRAPHER);
  if (isCommandActive(cart_status))
  {
    return std::string(KEY_CARTOGRAPHER);
  }

  return std::string();
}

RobotServiceClient::CommandStatus RobotServiceClient::getCommandStatus(const std::string& key) const
{
  const auto it = command_states_.find(key);
  if (it == command_states_.end())
  {
    return RobotServiceClient::CommandStatus::IDLE;
  }

  return it->second.status;
}

void RobotServiceClient::setRequestCallback(const std::function<void(bool)>& callback) { request_callback_ = callback; }

bool RobotServiceClient::computePathToPose(double x, double y, double theta)
{
  if (!initialized_ || !parent_node_)
  {
    qCritical() << "[RobotServiceClient::computePathToPose] RobotServiceClient is not initialized";
    return false;
  }

  if (!ac_compute_path_to_pose_)
  {
    qCritical() << "[RobotServiceClient::computePathToPose] Action client not available (robot not selected?)";
    return false;
  }

  if (!ac_compute_path_to_pose_->wait_for_action_server(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[RobotServiceClient::computePathToPose] Action server not available after waiting";
    return false;
  }

  nav2_msgs::action::ComputePathToPose::Goal goal_msg;
  goal_msg.goal.header.frame_id = context_ ? context_->resolveFrame(ROBOGait::ros::topics::TF_MAP_FRAME) : ROBOGait::ros::topics::TF_MAP_FRAME;
  goal_msg.goal.header.stamp = parent_node_->now();
  goal_msg.goal.pose.position.x = x;
  goal_msg.goal.pose.position.y = y;
  goal_msg.goal.pose.position.z = 0.0;
  goal_msg.goal.pose.orientation = ROBOGait::map::utils::createQuaternionFromYaw(theta);

  rclcpp_action::Client<nav2_msgs::action::ComputePathToPose>::SendGoalOptions options;
  options.result_callback = std::bind(&RobotServiceClient::resultComputePathToPoseCallback, this, std::placeholders::_1);

  manual_follow_path_.reset();
  ac_compute_path_to_pose_->async_send_goal(goal_msg, options);
  return true;
}

bool RobotServiceClient::navigateToPose(double x, double y, double theta)
{
  if (!initialized_ || !parent_node_)
  {
    qCritical() << "[RobotServiceClient::navigateToPose] RobotServiceClient is not initialized";
    return false;
  }

  if (!ac_navigate_to_pose_)
  {
    qCritical() << "[RobotServiceClient::navigateToPose] Action client not available (robot not selected?)";
    return false;
  }

  if (!ac_navigate_to_pose_->wait_for_action_server(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[RobotServiceClient::navigateToPose] Action server not available after waiting";
    return false;
  }

  nav2_msgs::action::NavigateToPose::Goal goal_msg;
  goal_msg.pose.header.frame_id = context_ ? context_->resolveFrame(ROBOGait::ros::topics::TF_MAP_FRAME) : ROBOGait::ros::topics::TF_MAP_FRAME;
  goal_msg.pose.header.stamp = parent_node_->now();
  goal_msg.pose.pose.position.x = x;
  goal_msg.pose.pose.position.y = y;
  goal_msg.pose.pose.position.z = 0.0;
  goal_msg.pose.pose.orientation = ROBOGait::map::utils::createQuaternionFromYaw(theta);

  if (cancel_in_progress_)
  {
    qInfo() << "[RobotServiceClient::navigateToPose] Cancel in progress, waiting for cancel to complete before sending new goal";
    return false;
  }

  if (nav_goal_active_)
  {
    cancelNavigateToPose();
    return false;
  }

  nav_goal_handle_.reset();

  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions options;
  options.goal_response_callback = std::bind(&RobotServiceClient::goalResponseNavigateToPoseCallback, this, std::placeholders::_1);
  options.result_callback = std::bind(&RobotServiceClient::resultNavigateToPoseCallback, this, std::placeholders::_1);

  ac_navigate_to_pose_->async_send_goal(goal_msg, options);
  return true;
}

bool RobotServiceClient::setManualFollowPath(const QVariantList& points)
{
  if (!initialized_ || !parent_node_)
  {
    qCritical() << "[RobotServiceClient::setManualFollowPath] RobotServiceClient is not initialized";
    return false;
  }

  manual_follow_path_.reset();

  if (points.size() < ROBOGait::map::interaction::geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    qCritical() << "[RobotServiceClient::setManualFollowPath] At least two points are required";
    return false;
  }

  const std::vector<ROBOGait::map::utils::WaypointInput> waypoints = ROBOGait::map::utils::parseWaypointInputs(points);

  if (waypoints.size() < ROBOGait::map::interaction::geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    qCritical() << "[RobotServiceClient::setManualFollowPath] Not enough valid points to build path";
    return false;
  }

  const std::string frame = context_ ? context_->resolveFrame(ROBOGait::ros::topics::TF_MAP_FRAME) : ROBOGait::ros::topics::TF_MAP_FRAME;

  nav_msgs::msg::Path path;
  path.header.frame_id = frame;

  if (useNavigationSimTime())
  {
    path.header.stamp.sec = 0;
    path.header.stamp.nanosec = 0;
  }
  else
  {
    const rclcpp::Time stamp = parent_node_->now();
    path.header.stamp = stamp;
  }
  path.poses.reserve(waypoints.size());

  for (size_t i = 0; i < waypoints.size(); ++i)
  {
    geometry_msgs::msg::PoseStamped pose;
    pose.header = path.header;
    pose.pose.position.x = waypoints[i].x;
    pose.pose.position.y = waypoints[i].y;
    pose.pose.position.z = 0.0;
    pose.pose.orientation = ROBOGait::map::utils::buildWaypointOrientation(waypoints[i]);

    path.poses.push_back(std::move(pose));
  }

  manual_follow_path_ = path;
  return true;
}

bool RobotServiceClient::followLastComputedPath()
{
  if (!initialized_ || !parent_node_)
  {
    qCritical() << "[RobotServiceClient::followLastComputedPath] RobotServiceClient is not initialized";
    return false;
  }

  if (!ac_follow_path_)
  {
    qCritical() << "[RobotServiceClient::followLastComputedPath] Action client not available (robot not selected?)";
    return false;
  }

  if (!manual_follow_path_.has_value())
  {
    qCritical() << "[RobotServiceClient::followLastComputedPath] No manual path available";
    return false;
  }

  if (manual_follow_path_->poses.size() < ROBOGait::map::interaction::geometry::StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    qCritical() << "[RobotServiceClient::followLastComputedPath] Manual path has not enough poses";
    return false;
  }

  if (!ac_follow_path_->wait_for_action_server(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[RobotServiceClient::followLastComputedPath] Action server not available after waiting";
    return false;
  }

  if (cancel_in_progress_)
  {
    qInfo() << "[RobotServiceClient::followLastComputedPath] Cancel in progress, waiting for cancel to complete before sending new goal";
    return false;
  }

  if (nav_goal_active_)
  {
    cancelNavigateToPose();
    return false;
  }

  nav2_msgs::action::FollowPath::Goal goal_msg;
  goal_msg.path = *manual_follow_path_;
  goal_msg.controller_id = getConfiguredNavigationId(CONFIG_MANUAL_FOLLOW_PATH_CONTROLLER_ID, DEFAULT_MANUAL_FOLLOW_PATH_CONTROLLER_ID);
  goal_msg.goal_checker_id = getConfiguredNavigationId(CONFIG_MANUAL_FOLLOW_PATH_GOAL_CHECKER_ID, DEFAULT_FOLLOW_PATH_GOAL_CHECKER_ID);

  nav_goal_handle_.reset();
  follow_path_goal_handle_.reset();

  rclcpp_action::Client<nav2_msgs::action::FollowPath>::SendGoalOptions options;
  options.goal_response_callback = std::bind(&RobotServiceClient::goalResponseFollowPathCallback, this, std::placeholders::_1);
  options.result_callback = std::bind(&RobotServiceClient::resultFollowPathCallback, this, std::placeholders::_1);

  ac_follow_path_->async_send_goal(goal_msg, options);
  return true;
}

bool RobotServiceClient::cancelNavigateToPose()
{
  if (!initialized_ || !parent_node_)
  {
    qCritical() << "[RobotServiceClient::cancelNavigateToPose] RobotServiceClient is not initialized";
    return false;
  }

  if (!ac_navigate_to_pose_ && !ac_follow_path_)
  {
    qCritical() << "[RobotServiceClient::cancelNavigateToPose] Action clients not available (robot not selected?)";
    return false;
  }

  if (!nav_goal_active_)
  {
    cancel_in_progress_ = true;
    nav_goal_handle_.reset();

    if (ac_navigate_to_pose_)
    {
      ac_navigate_to_pose_->async_cancel_all_goals(std::bind(&RobotServiceClient::cancelNavigateToPoseCallback, this, std::placeholders::_1));
    }

    if (ac_follow_path_)
    {
      ac_follow_path_->async_cancel_all_goals(std::bind(&RobotServiceClient::cancelFollowPathCallback, this, std::placeholders::_1));
    }
    return true;
  }

  cancel_in_progress_ = true;

  if (nav_goal_handle_)
  {
    if (ac_navigate_to_pose_)
    {
      ac_navigate_to_pose_->async_cancel_goal(nav_goal_handle_, std::bind(&RobotServiceClient::cancelNavigateToPoseCallback, this, std::placeholders::_1));
    }
  }
  else
  {
    if (ac_navigate_to_pose_)
    {
      ac_navigate_to_pose_->async_cancel_all_goals(std::bind(&RobotServiceClient::cancelNavigateToPoseCallback, this, std::placeholders::_1));
    }
  }

  if (follow_path_goal_handle_)
  {
    if (ac_follow_path_)
    {
      ac_follow_path_->async_cancel_goal(follow_path_goal_handle_, std::bind(&RobotServiceClient::cancelFollowPathCallback, this, std::placeholders::_1));
    }
  }
  else
  {
    if (ac_follow_path_)
    {
      ac_follow_path_->async_cancel_all_goals(std::bind(&RobotServiceClient::cancelFollowPathCallback, this, std::placeholders::_1));
    }
  }

  return true;
}

void RobotServiceClient::setPathResultCallback(const std::function<void(bool, const nav_msgs::msg::Path&)>& callback) { path_result_callback_ = callback; }

void RobotServiceClient::setNavigationResultCallback(const std::function<void(NavigationResult)>& callback) { navigation_result_callback_ = callback; }

void RobotServiceClient::resetRobotServiceClient()
{
  clearRobotContext();
  context_.reset();
  initialized_ = false;
  commands_.clear();
  command_states_.clear();
  cb_group_.reset();
  parent_node_ = nullptr;
}

bool RobotServiceClient::loadCommands()
{

  auto& bootstrap_loader = ROBOGait::loader::BootStrapLoader::getInstance();
  const auto bootstrap_config = bootstrap_loader.getBootStrapConfig();

  const std::filesystem::path commands_path =
      std::filesystem::path(QDir::homePath().toStdString()) / bootstrap_config.user_config_root_path / bootstrap_config.commands_file_name;

  if (!std::filesystem::exists(commands_path))
  {
    qCritical() << "[RobotServiceClient::loadCommands]  Commands file does not exist:" << QString::fromStdString(commands_path);
    return false;
  }

  YAML::Node config = YAML::LoadFile(commands_path);

  if (!config)
  {
    qCritical() << "[RobotServiceClient::loadCommands] Failed to load commands from " << QString::fromStdString(commands_path);
    return false;
  }

  for (const auto& it : config)
  {
    const std::string key = it.first.as<std::string>();
    const YAML::Node node = it.second;

    RobotServiceClient::CommandInfo info;

    info.name = node["name"].as<std::string>();
    info.cmd = node["cmd"].as<std::string>();
    info.append_args = node["append_args"].as<std::string>("");

    if (!info.cmd.empty())
    {
      commands_[key] = info;
    }
  }

  return !commands_.empty();
}

bool RobotServiceClient::loadTimeouts()
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();
  const int default_start_stop_timeout_s = static_cast<int>(START_STOP_TIMEOUT.count());
  const int configured_start_stop_timeout_s = yaml_loader.getValue<int>("timeouts.start_stop_timeout_s", default_start_stop_timeout_s);
  const int sanitized_start_stop_timeout_s = std::max(1, configured_start_stop_timeout_s);

  start_stop_timeout_s_ = std::chrono::seconds(sanitized_start_stop_timeout_s);
  return true;
}

std::string RobotServiceClient::getConfiguredNavigationId(std::string_view key, std::string_view default_value) const
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    return std::string(default_value);
  }

  const std::string key_string(key);
  const std::string default_string(default_value);
  return yaml_loader.getValue<std::string>(key_string, default_string);
}

bool RobotServiceClient::useNavigationSimTime() const
{
  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[RobotServiceClient::useNavigationSimTime] YAML loader is not loaded";
    return false;
  }

  const std::string key_string(CONFIG_NAVIGATION_USE_SIM_TIME);
  return yaml_loader.getValue<bool>(key_string, false);
}

bool RobotServiceClient::callCommandServiceAsync(const std::string& cmd, bool execute, const CommandRequestContext& context)
{
  if (!initialized_)
  {
    qCritical() << "[RobotServiceClient::callCommandServiceAsync] RobotServiceClient is not initialized";
    handleCommandResponse(context, false);
    return false;
  }

  if (!cli_cmd_)
  {
    qCritical() << "[RobotServiceClient::callCommandServiceAsync] Command client is not available (robot not selected?)";
    handleCommandResponse(context, false);
    return false;
  }

  if (!cli_cmd_->wait_for_service(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[RobotServiceClient::callCommandServiceAsync] Command service is not available after waiting";
    handleCommandResponse(context, false);
    return false;
  }

  auto request = std::make_shared<command_executor_msgs::srv::Cmd::Request>();
  request->cmd = cmd;
  request->execute = execute;

  const auto ctx = context;
  cli_cmd_->async_send_request(request,
                               [this, ctx](rclcpp::Client<command_executor_msgs::srv::Cmd>::SharedFuture future)
                               {
                                 bool ok = false;
                                 auto response = future.get();
                                 if (!response)
                                 {
                                   qCritical() << "[RobotServiceClient::callCommandServiceAsync] Failed to get response from command service";
                                   ok = false;
                                 }

                                 ok = response->success;

                                 handleCommandResponse(ctx, ok);
                               });

  return true;
}

void RobotServiceClient::handleCommandResponse(const CommandRequestContext& context, bool success)
{
  switch (context.type)
  {
    case CommandRequestType::StartMapping:
      handleStartMappingResult(success, context.full_cmd);
      break;
    case CommandRequestType::DeleteMap:
      handleDeleteMapResult(success, context.map_name);
      break;
    case CommandRequestType::StartNavigation:
      handleStartNavigationResult(success, context.full_cmd, context.map_name);
      break;
    case CommandRequestType::SaveMap:
      handleSaveMapResult(success, context.full_cmd);
      break;
    case CommandRequestType::StopCommand:
      handleStopCommandResult(success, context.command_key, context.full_cmd);
      break;
    default:
      notifyRequestResult(success);
      break;
  }
}

void RobotServiceClient::handleStartMappingResult(bool success, const std::string& full_cmd)
{
  if (!success)
  {
    qCritical() << "[RobotServiceClient::startMapping] Failed to initialize mapping";
    setCommandState(KEY_CARTOGRAPHER, RobotServiceClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(KEY_CARTOGRAPHER, RobotServiceClient::CommandStatus::STARTING, full_cmd);
  startHealthTimer();
  qInfo() << "[RobotServiceClient::startMapping] Successfully initialized mapping";
  notifyRequestResult(true);
}

void RobotServiceClient::handleDeleteMapResult(bool success, const std::string& map_name)
{
  if (!success)
  {
    qCritical() << "[RobotServiceClient::deleteMap] Failed to delete map";
    notifyRequestResult(false);
    return;
  }

  qInfo() << "[RobotServiceClient::deleteMap] Successfully deleted map " << QString::fromStdString(map_name);
  notifyRequestResult(true);
}

void RobotServiceClient::handleStartNavigationResult(bool success, const std::string& full_cmd, const std::string& map_name)
{
  if (!success)
  {
    qCritical() << "[RobotServiceClient::startNavigation] Failed to start navigation";
    setCommandState(KEY_NAVIGATION, RobotServiceClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(KEY_NAVIGATION, RobotServiceClient::CommandStatus::STARTING, full_cmd);
  startHealthTimer();
  qInfo() << "[RobotServiceClient::startNavigation] Successfully started navigation with map " << QString::fromStdString(map_name);
  notifyRequestResult(true);
}

void RobotServiceClient::handleSaveMapResult(bool success, const std::string& full_cmd)
{
  if (!success)
  {
    qCritical() << "[RobotServiceClient::saveMap] Failed to save map";
    setCommandState(KEY_MAP_SAVER, RobotServiceClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(KEY_MAP_SAVER, RobotServiceClient::CommandStatus::STARTING, full_cmd);
  startHealthTimer();
  qInfo() << "[RobotServiceClient::saveMap] Successfully saved map";
  notifyRequestResult(true);
}

void RobotServiceClient::handleStopCommandResult(bool success, const std::string& key, const std::string& full_cmd)
{
  if (!success)
  {
    qCritical() << "[RobotServiceClient::stopCommand] Failed to stop command for key: " << QString::fromStdString(key);
    setCommandState(key, RobotServiceClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(key, RobotServiceClient::CommandStatus::STOPPING, full_cmd);
  startHealthTimer();
  notifyRequestResult(true);
}

void RobotServiceClient::handleGlobalLocalizationResult(bool success)
{
  if (!success)
  {
    qCritical() << "[RobotServiceClient::reinitializeGlobalLocalization] Failed to call global localization service";
    notifyRequestResult(false);
    return;
  }

  qInfo() << "[RobotServiceClient::reinitializeGlobalLocalization] Global localization request sent";
  notifyRequestResult(true);
}

void RobotServiceClient::notifyRequestResult(bool success)
{
  if (request_callback_)
  {
    request_callback_(success);
  }
}

void RobotServiceClient::notifyNavigationResult(NavigationResult result)
{
  if (navigation_result_callback_)
  {
    navigation_result_callback_(result);
  }
}

bool RobotServiceClient::callGetMapDataService(const std::string& map_name, std::optional<std::string>& yaml_out, std::optional<std::vector<uint8_t>>& pgm_out)
{
  if (!initialized_)
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] RobotServiceClient is not initialized";
    return false;
  }

  if (!cli_get_map_data_)
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] GetMapData service client is not available (robot not selected?)";
    return false;
  }

  if (map_name.empty())
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] Map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] Map path is empty in YAML configuration";
    return false;
  }

  const std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);

  if (safe_name.empty())
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] Sanitized map name is empty";
    return false;
  }

  const std::string full_map_directory = "$HOME" + map_path;

  if (!cli_get_map_data_->wait_for_service(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] GetMapData service is not available after waiting";
    return false;
  }

  auto request = std::make_shared<command_executor_msgs::srv::GetMapData::Request>();
  request->map_path = full_map_directory;
  request->map_name = safe_name;

  auto future = cli_get_map_data_->async_send_request(request);
  std::future_status status = future.wait_for(ROBOGait::ros::define::SERVICE_CALL_TIMEOUT);

  if (status == std::future_status::ready && future.valid())
  {
    auto response = future.get();
    if (!response)
    {
      qCritical() << "[RobotServiceClient::callGetMapDataService] Failed to get response from GetMapData service";
      return false;
    }

    if (!response->success)
    {
      qCritical() << "[RobotServiceClient::callGetMapDataService] GetMapData service responded with failure: "
                  << QString::fromStdString(response->error_message);
      return false;
    }

    yaml_out = response->yaml_content;
    pgm_out = response->pgm_content;
    return true;
  }
  else
  {
    qCritical() << "[RobotServiceClient::callGetMapDataService] Error calling GetMapData service";
    return false;
  }
}

std::string RobotServiceClient::buildCommand(const std::string& cmd, const std::string& args)
{
  if (args.empty())
  {
    return cmd;
  }

  return cmd + " " + args;
}

bool RobotServiceClient::validateCommandKey(const std::string& key) const
{
  const bool exist = commands_.find(key) != commands_.end();

  if (!exist)
  {
    qWarning() << "[RobotServiceClient::validateCommandKey] Command key not found:" << QString::fromStdString(key);
  }

  return exist;
}

std::string RobotServiceClient::resolveServiceName(const std::string& service_name) const
{
  if (service_name.empty())
  {
    qCritical() << "[RobotServiceClient::resolveServiceName] Service name is empty";
    return service_name;
  }

  const std::string base = service_name;

  if (context_)
  {
    return context_->resolveTopic(base);
  }

  if (base.empty() || base.front() == '/')
  {
    return base;
  }

  return "/" + base;
}

bool RobotServiceClient::rebuildClient()
{
  if (!parent_node_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Parent node is null";
    return false;
  }

  if (!cb_group_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Callback group is null";
    return false;
  }

  cli_cmd_ = parent_node_->create_client<command_executor_msgs::srv::Cmd>(resolveServiceName(ROBOGait::ros::topics::S_CMD),
                                                                          ROBOGait::ros::QosProfiles::QOS_CLIENTS(), cb_group_);

  if (!cli_cmd_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Failed to create command client";
    return false;
  }

  cli_get_map_data_ = parent_node_->create_client<command_executor_msgs::srv::GetMapData>(resolveServiceName(ROBOGait::ros::topics::S_GET_MAP_DATA),
                                                                                          ROBOGait::ros::QosProfiles::QOS_CLIENTS(), cb_group_);

  if (!cli_get_map_data_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Failed to create get_map_data client";
    cli_cmd_.reset();
    return false;
  }

  cli_global_localization_ = parent_node_->create_client<std_srvs::srv::Empty>(resolveServiceName(ROBOGait::ros::topics::S_REINITIALIZE_GLOBAL_LOCALIZATION),
                                                                               ROBOGait::ros::QosProfiles::QOS_CLIENTS(), cb_group_);

  if (!cli_global_localization_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Failed to create global localization client";
    cli_cmd_.reset();
    cli_get_map_data_.reset();
    return false;
  }

  ac_compute_path_to_pose_ = rclcpp_action::create_client<nav2_msgs::action::ComputePathToPose>(
      parent_node_->get_node_base_interface(), parent_node_->get_node_graph_interface(), parent_node_->get_node_logging_interface(),
      parent_node_->get_node_waitables_interface(), resolveServiceName(ROBOGait::ros::topics::A_COMPUTE_PATH_TO_POSE), cb_group_);

  if (!ac_compute_path_to_pose_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Failed to create compute_path_to_pose action client";
    cli_cmd_.reset();
    cli_get_map_data_.reset();
    cli_global_localization_.reset();
    return false;
  }

  ac_navigate_to_pose_ = rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(
      parent_node_->get_node_base_interface(), parent_node_->get_node_graph_interface(), parent_node_->get_node_logging_interface(),
      parent_node_->get_node_waitables_interface(), resolveServiceName(ROBOGait::ros::topics::A_NAVIGATE_TO_POSE), cb_group_);

  if (!ac_navigate_to_pose_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Failed to create navigate_to_pose action client";
    cli_cmd_.reset();
    cli_get_map_data_.reset();
    cli_global_localization_.reset();
    ac_compute_path_to_pose_.reset();
    return false;
  }

  ac_follow_path_ = rclcpp_action::create_client<nav2_msgs::action::FollowPath>(
      parent_node_->get_node_base_interface(), parent_node_->get_node_graph_interface(), parent_node_->get_node_logging_interface(),
      parent_node_->get_node_waitables_interface(), resolveServiceName(ROBOGait::ros::topics::A_FOLLOW_PATH), cb_group_);

  if (!ac_follow_path_)
  {
    qCritical() << "[RobotServiceClient::rebuildClient] Failed to create follow_path action client";
    cli_cmd_.reset();
    cli_get_map_data_.reset();
    cli_global_localization_.reset();
    ac_compute_path_to_pose_.reset();
    ac_navigate_to_pose_.reset();
    return false;
  }

  return true;
}

bool RobotServiceClient::saveMap(const std::string& map_name)
{

  if (map_name.empty())
  {
    qCritical() << "[RobotServiceClient::saveMap] Map name is empty";
    return false;
  }

  const std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);

  if (safe_name.empty())
  {
    qCritical() << "[RobotServiceClient::saveMap] Sanitized map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[RobotServiceClient::saveMap] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[RobotServiceClient::saveMap] Map path is empty in YAML configuration";
    return false;
  }

  if (!validateCommandKey(KEY_MAP_SAVER))
  {
    qCritical() << "[RobotServiceClient::saveMap] Command key" << KEY_MAP_SAVER << "is not valid";
    return false;
  }

  const RobotServiceClient::CommandInfo& cmd_info = commands_[KEY_MAP_SAVER];

  std::string map_topic = context_ ? context_->resolveTopic(ROBOGait::ros::topics::T_MAP) : ROBOGait::ros::topics::T_MAP;

  std::string args = cmd_info.append_args;

  const std::unordered_map<std::string, std::string> vars = {
      {"{map_topic}", map_topic},
      {"{map_path}", map_path},
      {"{map_name}", safe_name},
  };

  args = replacePlaceholders(args, vars);

  const std::string full_cmd = buildCommand(cmd_info.cmd, args);

  CommandRequestContext context{CommandRequestType::SaveMap, std::string(KEY_MAP_SAVER), full_cmd, std::string()};

  const bool sent = callCommandServiceAsync(full_cmd, true, context);

  if (!sent)
  {
    qCritical() << "[RobotServiceClient::saveMap] Failed to save map";
    return false;
  }

  return true;
}

bool RobotServiceClient::stopCommand(const std::string& key)
{
  if (!validateCommandKey(key))
  {
    qCritical() << "[RobotServiceClient::stopCommand] Command key not valid: " << QString::fromStdString(key);
    return false;
  }

  const RobotServiceClient::CommandInfo& cmd_info = commands_[key];

  std::string full_cmd;
  const auto state_it = command_states_.find(key);
  if (state_it != command_states_.end() && !state_it->second.full_cmd.empty())
  {
    full_cmd = state_it->second.full_cmd;
  }
  else
  {
    full_cmd = buildCommand(cmd_info.cmd, cmd_info.append_args);
  }

  if (full_cmd.empty())
  {
    qCritical() << "[RobotServiceClient::stopCommand] Full command is empty for key: " << QString::fromStdString(key);
    setCommandState(key, RobotServiceClient::CommandStatus::ERROR);
    return false;
  }

  CommandRequestContext context{CommandRequestType::StopCommand, key, full_cmd, std::string()};

  const bool sent = callCommandServiceAsync(full_cmd, false, context);

  if (!sent)
  {
    qCritical() << "[RobotServiceClient::stopCommand] Failed to stop command for key: " << QString::fromStdString(key);
    setCommandState(key, RobotServiceClient::CommandStatus::ERROR, full_cmd);
    return false;
  }

  return true;
}

void RobotServiceClient::setCommandState(const std::string& key, RobotServiceClient::CommandStatus status, const std::string& full_cmd)
{
  auto& state = command_states_[key];
  if (!full_cmd.empty())
  {
    state.full_cmd = full_cmd;
  }

  if (state.status != status)
  {
    state.status = status;
    state.timestamp = std::chrono::steady_clock::now();
  }
}

bool RobotServiceClient::isCommandActive(RobotServiceClient::CommandStatus status) const
{
  return status == RobotServiceClient::CommandStatus::STARTING || status == RobotServiceClient::CommandStatus::RUNNING ||
         status == RobotServiceClient::CommandStatus::STOPPING;
}

void RobotServiceClient::startHealthTimer()
{
  if (!parent_node_)
  {
    qCritical() << "[RobotServiceClient::startHealthTimer] Parent node is null";
    return;
  }

  if (timer_health_)
  {
    return;
  }

  timer_health_ = parent_node_->create_wall_timer(HEALTH_CHECK_PERIOD, std::bind(&RobotServiceClient::onHealthTimer, this)); // one-shot=false, auto-start=true
}

void RobotServiceClient::stopHealthTimer()
{
  if (timer_health_)
  {
    timer_health_->cancel();
    timer_health_.reset();
  }
}

void RobotServiceClient::onHealthTimer()
{
  if (command_states_.empty())
  {
    stopHealthTimer();
    return;
  }

  const auto now = std::chrono::steady_clock::now();
  std::vector<std::string> keys;
  keys.reserve(command_states_.size());
  for (const auto& entry : command_states_)
  {
    keys.push_back(entry.first);
  }

  for (const auto& key : keys)
  {
    auto state_it = command_states_.find(key);
    if (state_it == command_states_.end())
    {
      continue;
    }

    RobotServiceClient::CommandState& state = state_it->second;
    if (state.status == RobotServiceClient::CommandStatus::IDLE || state.status == RobotServiceClient::CommandStatus::STOPPED ||
        state.status == RobotServiceClient::CommandStatus::ERROR)
    {
      continue;
    }

    const auto cmd_it = commands_.find(key);
    if (cmd_it == commands_.end())
    {
      qCritical() << "[RobotServiceClient::onHealthTimer] Command key not found in commands map: " << QString::fromStdString(key);
      setCommandState(key, RobotServiceClient::CommandStatus::ERROR);
      continue;
    }

    const std::string& node_name = cmd_it->second.name;
    if (node_name.empty())
    {
      qCritical() << "[RobotServiceClient::onHealthTimer] Node name is empty for command: " << QString::fromStdString(key);
      setCommandState(key, RobotServiceClient::CommandStatus::ERROR);
      continue;
    }

    const bool alive = isNodeAlive(node_name);
    const auto elapsed = now - state.timestamp;

    switch (state.status)
    {
      case RobotServiceClient::CommandStatus::STARTING:
        if (alive)
        {
          qInfo() << "[RobotServiceClient::onHealthTimer] Node is alive for command: " << QString::fromStdString(key);
          setCommandState(key, RobotServiceClient::CommandStatus::RUNNING, state.full_cmd);
        }
        else if (key == KEY_MAP_SAVER && elapsed > MAP_SAVER_STARTUP_ASSUME_STOP_DELAY)
        {
          setCommandState(key, RobotServiceClient::CommandStatus::STOPPED, state.full_cmd);
        }
        else if (elapsed > start_stop_timeout_s_)
        {
          setCommandState(key, RobotServiceClient::CommandStatus::ERROR, state.full_cmd);
        }
        break;
      case RobotServiceClient::CommandStatus::RUNNING:
        if (!alive)
        {
          setCommandState(key, RobotServiceClient::CommandStatus::STOPPED, state.full_cmd);
        }
        break;
      case RobotServiceClient::CommandStatus::STOPPING:
        if (!alive)
        {
          setCommandState(key, RobotServiceClient::CommandStatus::STOPPED, state.full_cmd);
        }
        else if (elapsed > start_stop_timeout_s_)
        {
          setCommandState(key, RobotServiceClient::CommandStatus::ERROR, state.full_cmd);
        }
        break;
      case RobotServiceClient::CommandStatus::STOPPED:
      case RobotServiceClient::CommandStatus::ERROR:
      case RobotServiceClient::CommandStatus::IDLE:
      default:
        break;
    }
  }

  if (pending_stop_after_save_)
  {
    const RobotServiceClient::CommandStatus saver_status = getCommandStatus(KEY_MAP_SAVER);
    if (saver_status == RobotServiceClient::CommandStatus::STOPPED || saver_status == RobotServiceClient::CommandStatus::ERROR ||
        saver_status == RobotServiceClient::CommandStatus::IDLE)
    {
      if (saver_status == RobotServiceClient::CommandStatus::ERROR && !map_saver_stop_requested_)
      {
        map_saver_stop_requested_ = true;
        stopCommand(KEY_MAP_SAVER);
      }

      pending_stop_after_save_ = false;
      stopCommand(KEY_CARTOGRAPHER);
    }
    else if (saver_status == RobotServiceClient::CommandStatus::RUNNING)
    {
      const auto saver_it = command_states_.find(KEY_MAP_SAVER);
      if (saver_it != command_states_.end() && !map_saver_stop_requested_)
      {
        const auto run_elapsed = now - saver_it->second.timestamp;
        if (run_elapsed > MAP_SAVER_FORCE_STOP_DELAY)
        {
          map_saver_stop_requested_ = true;
          stopCommand(KEY_MAP_SAVER);
        }
      }
    }
  }

  bool has_active = false;
  for (const auto& entry : command_states_)
  {
    const auto st = entry.second.status;
    if (st == RobotServiceClient::CommandStatus::STARTING || st == RobotServiceClient::CommandStatus::RUNNING ||
        st == RobotServiceClient::CommandStatus::STOPPING)
    {
      has_active = true;
      break;
    }
  }

  if (!has_active)
  {
    stopHealthTimer();
  }
}

std::string RobotServiceClient::replacePlaceholders(std::string input, const std::unordered_map<std::string, std::string>& values) const
{
  if (values.empty())
  {
    return input;
  }

  for (const auto& [placeholder, value] : values)
  {
    if (placeholder.empty())
    {
      continue;
    }

    const std::size_t placeholder_size = placeholder.size();
    const std::size_t value_size = value.size();

    std::size_t pos = 0;

    while ((pos = input.find(placeholder, pos)) != std::string::npos)
    {
      input.replace(pos, placeholder_size, value);
      pos += value_size;
    }
  }

  return input;
}

bool RobotServiceClient::publishMapDataOnce(const nav_msgs::msg::OccupancyGrid& occupancy_grid)
{
  if (!initialized_)
  {
    qCritical() << "[RobotServiceClient::publishMapDataOnce] RobotServiceClient is not initialized";
    return false;
  }

  if (!pub_map_data_)
  {
    std::string topic_name = context_ ? context_->resolveTopic(ROBOGait::ros::topics::T_MAP) : ROBOGait::ros::topics::T_MAP;
    pub_map_data_ = parent_node_->create_publisher<nav_msgs::msg::OccupancyGrid>(topic_name, ROBOGait::ros::QosProfiles::QOS_RELIABLE_LATCH());
    if (!pub_map_data_)
    {
      qCritical() << "[RobotServiceClient::publishMapDataOnce] Failed to create map data publisher";
      return false;
    }
  }

  pub_map_data_->publish(occupancy_grid);
  return true;
}

void RobotServiceClient::resultComputePathToPoseCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::ComputePathToPose>::WrappedResult& result)
{
  const bool success = (result.code == rclcpp_action::ResultCode::SUCCEEDED) && result.result;
  nav_msgs::msg::Path path;

  if (success)
  {
    path = result.result->path;
  }

  if (path_result_callback_)
  {
    path_result_callback_(success, path);
  }
}

void RobotServiceClient::goalResponseNavigateToPoseCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr& goal_handle)
{
  if (!goal_handle)
  {
    nav_goal_active_ = false;
    nav_goal_handle_.reset();
    qWarning() << "[RobotServiceClient::goalResponseNavigateToPoseCallback] Goal rejected by server";
    return;
  }

  nav_goal_handle_ = goal_handle;
  nav_goal_active_ = true;

  if (cancel_in_progress_)
  {
    ac_navigate_to_pose_->async_cancel_goal(nav_goal_handle_, std::bind(&RobotServiceClient::cancelNavigateToPoseCallback, this, std::placeholders::_1));
  }
}

void RobotServiceClient::cancelNavigateToPoseCallback(typename rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::CancelResponse::SharedPtr response)
{
  if (response && response->return_code == action_msgs::srv::CancelGoal::Response::ERROR_NONE)
  {
    qInfo() << "[RobotServiceClient::cancelNavigateToPoseCallback] Navigation cancel accepted";

    nav_goal_active_ = false;

    if (response->goals_canceling.empty())
    {
      qInfo() << "[RobotServiceClient::cancelNavigateToPoseCallback] No goals active to cancel";
    }
  }
  else
  {
    qWarning() << "[RobotServiceClient::cancelNavigateToPoseCallback] Failed to cancel navigation";
  }

  cancel_in_progress_ = false;
}

void RobotServiceClient::resultNavigateToPoseCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult& result)
{
  if (!nav_goal_handle_)
  {
    qWarning() << "[RobotServiceClient::resultNavigateToPoseCallback] Received result for inactive goal";
    return;
  }

  NavigationResult nav_result = NavigationResult::UNKNOWN;
  switch (result.code)
  {
    case rclcpp_action::ResultCode::SUCCEEDED:
      qInfo() << "[RobotServiceClient::resultNavigateToPoseCallback] Goal reached";
      nav_result = NavigationResult::SUCCEEDED;
      break;

    case rclcpp_action::ResultCode::CANCELED:
      qInfo() << "[RobotServiceClient::resultNavigateToPoseCallback] Goal canceled";
      nav_result = NavigationResult::CANCELED;
      break;

    case rclcpp_action::ResultCode::ABORTED:
      qWarning() << "[RobotServiceClient::resultNavigateToPoseCallback] Goal aborted";
      nav_result = NavigationResult::ABORTED;
      break;

    default:
      qWarning() << "[RobotServiceClient::resultNavigateToPoseCallback] Unknown state";
      nav_result = NavigationResult::UNKNOWN;
      break;
  }

  nav_goal_active_ = false;
  cancel_in_progress_ = false;
  nav_goal_handle_.reset();
  notifyNavigationResult(nav_result);
}

void RobotServiceClient::goalResponseFollowPathCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::FollowPath>::SharedPtr& goal_handle)
{
  if (!goal_handle)
  {
    nav_goal_active_ = false;
    follow_path_goal_handle_.reset();
    qWarning() << "[RobotServiceClient::goalResponseFollowPathCallback] Goal rejected by server";
    return;
  }

  follow_path_goal_handle_ = goal_handle;
  nav_goal_active_ = true;

  if (cancel_in_progress_)
  {
    ac_follow_path_->async_cancel_goal(follow_path_goal_handle_, std::bind(&RobotServiceClient::cancelFollowPathCallback, this, std::placeholders::_1));
  }
}

void RobotServiceClient::cancelFollowPathCallback(typename rclcpp_action::Client<nav2_msgs::action::FollowPath>::CancelResponse::SharedPtr response)
{
  if (response && response->return_code == action_msgs::srv::CancelGoal::Response::ERROR_NONE)
  {
    qInfo() << "[RobotServiceClient::cancelFollowPathCallback] Follow path cancel accepted";

    nav_goal_active_ = false;
    follow_path_goal_handle_.reset();

    if (response->goals_canceling.empty())
    {
      qInfo() << "[RobotServiceClient::cancelFollowPathCallback] No follow path goals active to cancel";
    }
  }
  else
  {
    qWarning() << "[RobotServiceClient::cancelFollowPathCallback] Failed to cancel follow path";
  }

  cancel_in_progress_ = false;
}

void RobotServiceClient::resultFollowPathCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::FollowPath>::WrappedResult& result)
{
  NavigationResult nav_result = NavigationResult::UNKNOWN;
  switch (result.code)
  {
    case rclcpp_action::ResultCode::SUCCEEDED:
      qInfo() << "[RobotServiceClient::resultFollowPathCallback] Goal reached";
      nav_result = NavigationResult::SUCCEEDED;
      break;

    case rclcpp_action::ResultCode::CANCELED:
      qInfo() << "[RobotServiceClient::resultFollowPathCallback] Goal canceled";
      nav_result = NavigationResult::CANCELED;
      break;

    case rclcpp_action::ResultCode::ABORTED:
      qWarning() << "[RobotServiceClient::resultFollowPathCallback] Goal aborted";
      nav_result = NavigationResult::ABORTED;
      break;

    default:
      qWarning() << "[RobotServiceClient::resultFollowPathCallback] Unknown state";
      nav_result = NavigationResult::UNKNOWN;
      break;
  }

  nav_goal_active_ = false;
  cancel_in_progress_ = false;
  follow_path_goal_handle_.reset();
  notifyNavigationResult(nav_result);
}
