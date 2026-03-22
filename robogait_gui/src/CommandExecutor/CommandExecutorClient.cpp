#include <ament_index_cpp/get_package_share_directory.hpp>
#include <filesystem>
#include <vector>
#include <yaml-cpp/yaml.h>

#include <QDebug>

#include "CommandExecutor/CommandExecutorClient.hpp"
#include "Loader/MapFileLoader.hpp"
#include "Loader/YamlLoader.hpp"
#include "Map/Utils/Utils.hpp"
#include "Ros/Define.hpp"
#include "Ros/TopicsName.hpp"

using namespace ROBOGait::ros::executor;

CommandExecutorClient& CommandExecutorClient::getInstance()
{
  static CommandExecutorClient instance;
  return instance;
}

CommandExecutorClient::CommandExecutorClient() :
    parent_node_(nullptr),
    timer_health_(nullptr),
    context_(std::nullopt),
    initialized_(false),
    pending_stop_after_save_(false),
    map_saver_stop_requested_(false)
{
}

bool CommandExecutorClient::initialize(rclcpp::Node* parent_node)
{
  if (initialized_)
  {
    qWarning() << "[CommandExecutorClient::initialize] CommandExecutorClient is already initialized";
    return true;
  }

  if (!parent_node)
  {
    qCritical() << "[CommandExecutorClient::initialize] Invalid parent_node";
    return false;
  }

  parent_node_ = parent_node;

  if (!loadCommands())
  {
    qCritical() << "[CommandExecutorClient::initialize] Failed to load commands";
    return false;
  }

  cb_group_ = parent_node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  if (!cb_group_)
  {
    qCritical() << "[CommandExecutorClient::initialize] Failed to create callback group";
    return false;
  }

  initialized_ = true;
  return initialized_;
}

bool CommandExecutorClient::isInitialized() const { return initialized_; }

void CommandExecutorClient::setRobotContext(const ROBOGait::context::RobotContext& context)
{
  context_ = context;

  if (initialized_)
  {
    if (!rebuildClient())
    {
      qCritical() << "[CommandExecutorClient::setRobotContext] Failed to rebuild command client";
    }
  }
}

void CommandExecutorClient::clearRobotContext()
{
  if (context_)
  {
    context_->clear();
  }
  cli_cmd_.reset();
  cli_get_map_data_.reset();
  pub_map_data_.reset();
  command_states_.clear();
  pending_stop_after_save_ = false;
  map_saver_stop_requested_ = false;
  stopHealthTimer();
}

bool CommandExecutorClient::startMapping()
{
  if (!validateCommandKey(KEY_CARTOGRAPHER))
  {
    qCritical() << "[CommandExecutorClient::startMapping] Command key" << KEY_CARTOGRAPHER << "is not valid";
    return false;
  }

  const CommandExecutorClient::CommandInfo& cmd_info = commands_[KEY_CARTOGRAPHER];

  const std::string full_cmd = buildCommand(cmd_info.cmd, cmd_info.append_args);

  pending_stop_after_save_ = false;
  map_saver_stop_requested_ = false;

  CommandRequestContext context{CommandRequestType::StartMapping, std::string(KEY_CARTOGRAPHER), full_cmd, std::string()};

  const bool sent = callCommandServiceAsync(full_cmd, true, context);

  if (!sent)
  {
    qCritical() << "[CommandExecutorClient::startMapping] Failed to initialize mapping";
    return false;
  }

  return true;
}

bool CommandExecutorClient::stopMapping(bool save_map, const std::string& map_name)
{
  if (save_map)
  {
    if (!saveMap(map_name))
    {
      qCritical() << "[CommandExecutorClient::stopMapping] Failed to start map saving";
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

bool CommandExecutorClient::deleteMap(const std::string& map_name)
{
  if (map_name.empty())
  {
    qCritical() << "[CommandExecutorClient::deleteMap] Map name is empty";
    return false;
  }

  const std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);
  if (safe_name.empty())
  {
    qCritical() << "[CommandExecutorClient::deleteMap] Sanitized map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[CommandExecutorClient::deleteMap] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[CommandExecutorClient::deleteMap] Map path is empty in YAML configuration";
    return false;
  }

  if (!validateCommandKey(KEY_DELETE_MAP))
  {
    qCritical() << "[CommandExecutorClient::deleteMap] Command key" << KEY_DELETE_MAP << "is not valid";
    return false;
  }

  const CommandExecutorClient::CommandInfo& cmd_info = commands_[KEY_DELETE_MAP];

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
    qCritical() << "[CommandExecutorClient::deleteMap] Failed to delete map";
    return false;
  }

  return true;
}

bool CommandExecutorClient::requestMapData(const std::string& map_name)
{
  if (map_name.empty())
  {
    qCritical() << "[CommandExecutorClient::requestMapData] Map name is empty";
    return false;
  }

  std::optional<std::string> yaml_info;
  std::optional<std::vector<uint8_t>> pgm_info;

  if (!callGetMapDataService(map_name, yaml_info, pgm_info))
  {
    qCritical() << "[CommandExecutorClient::requestMapData] Failed to call get map data service for map " << QString::fromStdString(map_name);
    return false;
  }

  ROBOGait::loader::MapFileLoader map_loader;

  auto occupancy_grid_opt = map_loader.loadMap(*yaml_info, *pgm_info);

  if (!occupancy_grid_opt)
  {
    qCritical() << "[CommandExecutorClient::requestMapData] Failed to load map data for map " << QString::fromStdString(map_name);
    return false;
  }

  auto occupancy_grid = occupancy_grid_opt.value();
  occupancy_grid.header.frame_id = context_ ? context_->resolveFrame(std::string(TF_MAP_FRAME)) : std::string(TF_MAP_FRAME);

  return publishMapDataOnce(occupancy_grid);
}

bool CommandExecutorClient::startNavigation(const std::string& map_name)
{
  if (map_name.empty())
  {
    qCritical() << "[CommandExecutorClient::startNavigation] Map name is empty";
    return false;
  }

  std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);
  if (safe_name.empty())
  {
    qCritical() << "[CommandExecutorClient::startNavigation] Sanitized map name is empty";
    return false;
  }

  safe_name = safe_name + ".yaml";

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[CommandExecutorClient::startNavigation] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[CommandExecutorClient::startNavigation] Map path is empty in YAML configuration";
    return false;
  }

  if (!validateCommandKey(KEY_NAVIGATION))
  {
    qCritical() << "[CommandExecutorClient::startNavigation] Command key" << KEY_NAVIGATION << "is not valid";
    return false;
  }

  const CommandExecutorClient::CommandInfo& cmd_info = commands_[KEY_NAVIGATION];
  std::string args = cmd_info.append_args;

  const std::unordered_map<std::string, std::string> vars = {
      {"{map_path}", map_path},
      {"{map_name}", safe_name},
  };

  args = replacePlaceholders(args, vars);

  const std::string full_cmd = buildCommand(cmd_info.cmd, args);

  CommandRequestContext context{CommandRequestType::StartNavigation, std::string(KEY_NAVIGATION), full_cmd, map_name};

  const bool sent = callCommandServiceAsync(full_cmd, true, context);

  if (!sent)
  {
    qCritical() << "[CommandExecutorClient::startNavigation] Failed to start navigation";
    return false;
  }

  return true;
}

bool CommandExecutorClient::stopNavigation()
{
  if (getCommandStatus(KEY_NAVIGATION) == CommandExecutorClient::CommandStatus::IDLE)
  {
    return true;
  }

  return stopCommand(KEY_NAVIGATION);
}

bool CommandExecutorClient::isNodeAlive(const std::string& node_name) const
{
  if (!parent_node_)
  {
    qCritical() << "[CommandExecutorClient::isNodeAlive] Parent node is not set";
    return false;
  }

  if (node_name.empty())
  {
    qCritical() << "[CommandExecutorClient::isNodeAlive] Node name is empty";
    return false;
  }

  auto graph = parent_node_->get_node_graph_interface();
  if (!graph)
  {
    qCritical() << "[CommandExecutorClient::isNodeAlive] Failed to get node graph interface";
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

CommandExecutorClient::CommandStatus CommandExecutorClient::getActiveCommandStatus() const
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

  if (cart_status != CommandExecutorClient::CommandStatus::IDLE)
  {
    return cart_status;
  }

  if (nav_status == CommandExecutorClient::CommandStatus::ERROR)
  {
    return nav_status;
  }

  return nav_status;
}

std::string CommandExecutorClient::getActiveCommandKey() const
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

CommandExecutorClient::CommandStatus CommandExecutorClient::getCommandStatus(const std::string& key) const
{
  const auto it = command_states_.find(key);
  if (it == command_states_.end())
  {
    return CommandExecutorClient::CommandStatus::IDLE;
  }

  return it->second.status;
}

void CommandExecutorClient::setRequestCallback(const std::function<void(bool)>& callback) { request_callback_ = callback; }

bool CommandExecutorClient::loadCommands()
{

  const std::filesystem::path share_path = ament_index_cpp::get_package_share_directory(ROBOGAIT_GUI);
  const std::string config_path = (share_path / "params" / "commands.yaml").string();

  YAML::Node config = YAML::LoadFile(config_path);

  if (!config)
  {
    qCritical() << "[CommandExecutorClient::loadCommands] Failed to load commands from " << QString::fromStdString(config_path);
    return false;
  }

  for (const auto& it : config)
  {
    const std::string key = it.first.as<std::string>();
    const YAML::Node node = it.second;

    CommandExecutorClient::CommandInfo info;

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

bool CommandExecutorClient::callCommandServiceAsync(const std::string& cmd, bool execute, const CommandRequestContext& context)
{
  if (!initialized_)
  {
    qCritical() << "[CommandExecutorClient::callCommandServiceAsync] CommandExecutorClient is not initialized";
    handleCommandResponse(context, false);
    return false;
  }

  if (!cli_cmd_)
  {
    qCritical() << "[CommandExecutorClient::callCommandServiceAsync] Command client is not available (robot not selected?)";
    handleCommandResponse(context, false);
    return false;
  }

  if (!cli_cmd_->wait_for_service(SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[CommandExecutorClient::callCommandServiceAsync] Command service is not available after waiting";
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
                                   qCritical() << "[CommandExecutorClient::callCommandServiceAsync] Failed to get response from command service";
                                   ok = false;
                                 }

                                 ok = response->success;

                                 handleCommandResponse(ctx, ok);
                               });

  return true;
}

void CommandExecutorClient::handleCommandResponse(const CommandRequestContext& context, bool success)
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

void CommandExecutorClient::handleStartMappingResult(bool success, const std::string& full_cmd)
{
  if (!success)
  {
    qCritical() << "[CommandExecutorClient::startMapping] Failed to initialize mapping";
    setCommandState(KEY_CARTOGRAPHER, CommandExecutorClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(KEY_CARTOGRAPHER, CommandExecutorClient::CommandStatus::STARTING, full_cmd);
  startHealthTimer();
  qInfo() << "[CommandExecutorClient::startMapping] Successfully initialized mapping";
  notifyRequestResult(true);
}

void CommandExecutorClient::handleDeleteMapResult(bool success, const std::string& map_name)
{
  if (!success)
  {
    qCritical() << "[CommandExecutorClient::deleteMap] Failed to delete map";
    notifyRequestResult(false);
    return;
  }

  qInfo() << "[CommandExecutorClient::deleteMap] Successfully deleted map " << QString::fromStdString(map_name);
  notifyRequestResult(true);
}

void CommandExecutorClient::handleStartNavigationResult(bool success, const std::string& full_cmd, const std::string& map_name)
{
  if (!success)
  {
    qCritical() << "[CommandExecutorClient::startNavigation] Failed to start navigation";
    setCommandState(KEY_NAVIGATION, CommandExecutorClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(KEY_NAVIGATION, CommandExecutorClient::CommandStatus::STARTING, full_cmd);
  startHealthTimer();
  qInfo() << "[CommandExecutorClient::startNavigation] Successfully started navigation with map " << QString::fromStdString(map_name);
  notifyRequestResult(true);
}

void CommandExecutorClient::handleSaveMapResult(bool success, const std::string& full_cmd)
{
  if (!success)
  {
    qCritical() << "[CommandExecutorClient::saveMap] Failed to save map";
    setCommandState(KEY_MAP_SAVER, CommandExecutorClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(KEY_MAP_SAVER, CommandExecutorClient::CommandStatus::STARTING, full_cmd);
  startHealthTimer();
  qInfo() << "[CommandExecutorClient::saveMap] Successfully saved map";
  notifyRequestResult(true);
}

void CommandExecutorClient::handleStopCommandResult(bool success, const std::string& key, const std::string& full_cmd)
{
  if (!success)
  {
    qCritical() << "[CommandExecutorClient::stopCommand] Failed to stop command for key: " << QString::fromStdString(key);
    setCommandState(key, CommandExecutorClient::CommandStatus::ERROR, full_cmd);
    notifyRequestResult(false);
    return;
  }

  setCommandState(key, CommandExecutorClient::CommandStatus::STOPPING, full_cmd);
  startHealthTimer();
  notifyRequestResult(true);
}

void CommandExecutorClient::notifyRequestResult(bool success)
{
  if (request_callback_)
  {
    request_callback_(success);
  }
}

bool CommandExecutorClient::callGetMapDataService(const std::string& map_name, std::optional<std::string>& yaml_out,
                                                  std::optional<std::vector<uint8_t>>& pgm_out)
{
  if (!initialized_)
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] CommandExecutorClient is not initialized";
    return false;
  }

  if (!cli_get_map_data_)
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] GetMapData service client is not available (robot not selected?)";
    return false;
  }

  if (map_name.empty())
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] Map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] Map path is empty in YAML configuration";
    return false;
  }

  const std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);

  if (safe_name.empty())
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] Sanitized map name is empty";
    return false;
  }

  const std::string full_map_directory = "$HOME" + map_path;

  if (!cli_get_map_data_->wait_for_service(SERVICE_CALL_TIMEOUT))
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] GetMapData service is not available after waiting";
    return false;
  }

  auto request = std::make_shared<command_executor_msgs::srv::GetMapData::Request>();
  request->map_path = full_map_directory;
  request->map_name = safe_name;

  auto future = cli_get_map_data_->async_send_request(request);
  std::future_status status = future.wait_for(SERVICE_CALL_TIMEOUT);

  if (status == std::future_status::ready && future.valid())
  {
    auto response = future.get();
    if (!response)
    {
      qCritical() << "[CommandExecutorClient::callGetMapDataService] Failed to get response from GetMapData service";
      return false;
    }

    if (!response->success)
    {
      qCritical() << "[CommandExecutorClient::callGetMapDataService] GetMapData service responded with failure: "
                  << QString::fromStdString(response->error_message);
      return false;
    }

    yaml_out = response->yaml_content;
    pgm_out = response->pgm_content;
    return true;
  }
  else
  {
    qCritical() << "[CommandExecutorClient::callGetMapDataService] Error calling GetMapData service";
    return false;
  }
}

std::string CommandExecutorClient::buildCommand(const std::string& cmd, const std::string& args)
{
  if (args.empty())
  {
    return cmd;
  }

  return cmd + " " + args;
}

bool CommandExecutorClient::validateCommandKey(const std::string& key) const
{
  const bool exist = commands_.find(key) != commands_.end();

  if (!exist)
  {
    qWarning() << "[CommandExecutorClient::validateCommandKey] Command key not found:" << QString::fromStdString(key);
  }

  return exist;
}

std::string CommandExecutorClient::resolveServiceName(const std::string& service_name) const
{
  if (service_name.empty())
  {
    qCritical() << "[CommandExecutorClient::resolveServiceName] Service name is empty";
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

bool CommandExecutorClient::rebuildClient()
{
  if (!parent_node_)
  {
    qCritical() << "[CommandExecutorClient::rebuildClient] Parent node is null";
    return false;
  }

  if (!cb_group_)
  {
    qCritical() << "[CommandExecutorClient::rebuildClient] Callback group is null";
    return false;
  }

  cli_cmd_ = parent_node_->create_client<command_executor_msgs::srv::Cmd>(resolveServiceName(std::string(S_CMD)), QOS_CLIENTS, cb_group_);

  if (!cli_cmd_)
  {
    qCritical() << "[CommandExecutorClient::rebuildClient] Failed to create command client";
    return false;
  }

  cli_get_map_data_ =
      parent_node_->create_client<command_executor_msgs::srv::GetMapData>(resolveServiceName(std::string(S_GET_MAP_DATA)), QOS_CLIENTS, cb_group_);

  if (!cli_get_map_data_)
  {
    qCritical() << "[CommandExecutorClient::rebuildClient] Failed to create get_map_data client";
    cli_cmd_.reset();
    return false;
  }

  return true;
}

bool CommandExecutorClient::saveMap(const std::string& map_name)
{

  if (map_name.empty())
  {
    qCritical() << "[CommandExecutorClient::saveMap] Map name is empty";
    return false;
  }

  const std::string safe_name = ROBOGait::map::utils::sanitizeMapName(map_name);

  if (safe_name.empty())
  {
    qCritical() << "[CommandExecutorClient::saveMap] Sanitized map name is empty";
    return false;
  }

  auto& yaml_loader = ROBOGait::loader::YamlLoader::getInstance();

  if (!yaml_loader.isLoaded())
  {
    qCritical() << "[CommandExecutorClient::saveMap] YAML loader is not loaded";
    return false;
  }

  const std::string map_path = yaml_loader.getValue<std::string>("map.map_path", "");

  if (map_path.empty())
  {
    qCritical() << "[CommandExecutorClient::saveMap] Map path is empty in YAML configuration";
    return false;
  }

  if (!validateCommandKey(KEY_MAP_SAVER))
  {
    qCritical() << "[CommandExecutorClient::saveMap] Command key" << KEY_MAP_SAVER << "is not valid";
    return false;
  }

  const CommandExecutorClient::CommandInfo& cmd_info = commands_[KEY_MAP_SAVER];

  std::string map_topic = context_ ? context_->resolveTopic(std::string(T_MAP)) : std::string(T_MAP);

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
    qCritical() << "[CommandExecutorClient::saveMap] Failed to save map";
    return false;
  }

  return true;
}

bool CommandExecutorClient::stopCommand(const std::string& key)
{
  if (!validateCommandKey(key))
  {
    qCritical() << "[CommandExecutorClient::stopCommand] Command key not valid: " << QString::fromStdString(key);
    return false;
  }

  const CommandExecutorClient::CommandInfo& cmd_info = commands_[key];

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
    qCritical() << "[CommandExecutorClient::stopCommand] Full command is empty for key: " << QString::fromStdString(key);
    setCommandState(key, CommandExecutorClient::CommandStatus::ERROR);
    return false;
  }

  CommandRequestContext context{CommandRequestType::StopCommand, key, full_cmd, std::string()};

  const bool sent = callCommandServiceAsync(full_cmd, false, context);

  if (!sent)
  {
    qCritical() << "[CommandExecutorClient::stopCommand] Failed to stop command for key: " << QString::fromStdString(key);
    setCommandState(key, CommandExecutorClient::CommandStatus::ERROR, full_cmd);
    return false;
  }

  return true;
}

void CommandExecutorClient::setCommandState(const std::string& key, CommandExecutorClient::CommandStatus status, const std::string& full_cmd)
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

bool CommandExecutorClient::isCommandActive(CommandExecutorClient::CommandStatus status) const
{
  return status == CommandExecutorClient::CommandStatus::STARTING || status == CommandExecutorClient::CommandStatus::RUNNING ||
         status == CommandExecutorClient::CommandStatus::STOPPING;
}

void CommandExecutorClient::startHealthTimer()
{
  if (!parent_node_)
  {
    qCritical() << "[CommandExecutorClient::startHealthTimer] Parent node is null";
    return;
  }

  if (timer_health_)
  {
    return;
  }

  timer_health_ =
      parent_node_->create_wall_timer(HEALTH_CHECK_PERIOD, std::bind(&CommandExecutorClient::onHealthTimer, this)); // one-shot=false, auto-start=true
}

void CommandExecutorClient::stopHealthTimer()
{
  if (timer_health_)
  {
    timer_health_->cancel();
    timer_health_.reset();
  }
}

void CommandExecutorClient::onHealthTimer()
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

    CommandExecutorClient::CommandState& state = state_it->second;
    if (state.status == CommandExecutorClient::CommandStatus::IDLE || state.status == CommandExecutorClient::CommandStatus::STOPPED ||
        state.status == CommandExecutorClient::CommandStatus::ERROR)
    {
      continue;
    }

    const auto cmd_it = commands_.find(key);
    if (cmd_it == commands_.end())
    {
      qCritical() << "[CommandExecutorClient::onHealthTimer] Command key not found in commands map: " << QString::fromStdString(key);
      setCommandState(key, CommandExecutorClient::CommandStatus::ERROR);
      continue;
    }

    const std::string& node_name = cmd_it->second.name;
    if (node_name.empty())
    {
      qCritical() << "[CommandExecutorClient::onHealthTimer] Node name is empty for command: " << QString::fromStdString(key);
      setCommandState(key, CommandExecutorClient::CommandStatus::ERROR);
      continue;
    }

    const bool alive = isNodeAlive(node_name);
    const auto elapsed = now - state.timestamp;

    switch (state.status)
    {
      case CommandExecutorClient::CommandStatus::STARTING:
        if (alive)
        {
          qInfo() << "[CommandExecutorClient::onHealthTimer] Node is alive for command: " << QString::fromStdString(key);
          setCommandState(key, CommandExecutorClient::CommandStatus::RUNNING, state.full_cmd);
        }
        else if (key == KEY_MAP_SAVER && elapsed > MAP_SAVER_STARTUP_ASSUME_STOP_DELAY)
        {
          setCommandState(key, CommandExecutorClient::CommandStatus::STOPPED, state.full_cmd);
        }
        else if (elapsed > START_STOP_TIMEOUT)
        {
          setCommandState(key, CommandExecutorClient::CommandStatus::ERROR, state.full_cmd);
        }
        break;
      case CommandExecutorClient::CommandStatus::RUNNING:
        if (!alive)
        {
          setCommandState(key, CommandExecutorClient::CommandStatus::STOPPED, state.full_cmd);
        }
        break;
      case CommandExecutorClient::CommandStatus::STOPPING:
        if (!alive)
        {
          setCommandState(key, CommandExecutorClient::CommandStatus::STOPPED, state.full_cmd);
        }
        else if (elapsed > START_STOP_TIMEOUT)
        {
          setCommandState(key, CommandExecutorClient::CommandStatus::ERROR, state.full_cmd);
        }
        break;
      case CommandExecutorClient::CommandStatus::STOPPED:
      case CommandExecutorClient::CommandStatus::ERROR:
      case CommandExecutorClient::CommandStatus::IDLE:
      default:
        break;
    }
  }

  if (pending_stop_after_save_)
  {
    const CommandExecutorClient::CommandStatus saver_status = getCommandStatus(KEY_MAP_SAVER);
    if (saver_status == CommandExecutorClient::CommandStatus::STOPPED || saver_status == CommandExecutorClient::CommandStatus::ERROR ||
        saver_status == CommandExecutorClient::CommandStatus::IDLE)
    {
      if (saver_status == CommandExecutorClient::CommandStatus::ERROR && !map_saver_stop_requested_)
      {
        map_saver_stop_requested_ = true;
        stopCommand(KEY_MAP_SAVER);
      }

      pending_stop_after_save_ = false;
      stopCommand(KEY_CARTOGRAPHER);
    }
    else if (saver_status == CommandExecutorClient::CommandStatus::RUNNING)
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
    if (st == CommandExecutorClient::CommandStatus::STARTING || st == CommandExecutorClient::CommandStatus::RUNNING ||
        st == CommandExecutorClient::CommandStatus::STOPPING)
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

std::string CommandExecutorClient::replacePlaceholders(std::string input, const std::unordered_map<std::string, std::string>& values) const
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

bool CommandExecutorClient::publishMapDataOnce(const nav_msgs::msg::OccupancyGrid& occupancy_grid)
{
  if (!initialized_)
  {
    qCritical() << "[CommandExecutorClient::publishMapDataOnce] CommandExecutorClient is not initialized";
    return false;
  }

  if (!pub_map_data_)
  {
    std::string topic_name = context_ ? context_->resolveTopic(std::string(T_MAP)) : std::string(T_MAP);
    pub_map_data_ = parent_node_->create_publisher<nav_msgs::msg::OccupancyGrid>(topic_name, QOS_RELIABLE_LATCH);
    if (!pub_map_data_)
    {
      qCritical() << "[CommandExecutorClient::publishMapDataOnce] Failed to create map data publisher";
      return false;
    }
  }

  pub_map_data_->publish(occupancy_grid);
  return true;
}
