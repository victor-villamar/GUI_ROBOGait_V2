#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <sstream>

#include <rclcpp/create_publisher.hpp>
#include <rclcpp/create_service.hpp>
#include <rclcpp/create_timer.hpp>

#include <ament_index_cpp/get_package_share_directory.hpp>

#include "CommandExecutor.hpp"
#include "Define.hpp"
#include "Functions.hpp"
#include "QoSProfile.hpp"

using namespace ROBOGait::command;

CommandExecutor::CommandExecutor(const rclcpp::NodeOptions& options) : Node(COMMAND_EXECUTOR, options)
{
  RCLCPP_INFO(get_logger(), "[CommandExecutor::CommandExecutor] CommandExecutor created");
}

CommandExecutor::~CommandExecutor() { RCLCPP_INFO(get_logger(), "[CommandExecutor::~CommandExecutor] CommandExecutor destroyed"); }

bool CommandExecutor::initialize()
{

  if (!loadConfig())
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::initialize] Failed to load config");
    return false;
  }

  if (!createRosInterfaces())
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::initialize] Failed to create ROS interfaces");
    return false;
  }

  RCLCPP_INFO(get_logger(), "[CommandExecutor::initialize] Initialization successful");
  return true;
}

void CommandExecutor::handleCommand(const std::shared_ptr<command_executor_msgs::srv::Cmd::Request>& request,
                                    std::shared_ptr<command_executor_msgs::srv::Cmd::Response> response)
{
  const auto& cmd = request->cmd;

  if (!isAllowedCommand(cmd))
  {
    RCLCPP_WARN(get_logger(), "[CommandExecutor::handleCommand] Rejected command '%s', not allowed", cmd.c_str());
    response->success = false;
    return;
  }

  // Handle delete command
  std::string delete_cmd;
  if (isDeleteCommand(cmd, delete_cmd))
  {
    bool ok = process_manager_.executeOneShotCommand(delete_cmd);
    RCLCPP_INFO(get_logger(), "[CommandExecutor::handleCommand] Delete command '%s' executed %s", delete_cmd.c_str(), ok ? "successfully" : "failed");
    response->success = ok;
    return;
  }

  bool ok = false;

  if (request->execute)
  {
    ok = process_manager_.startProcess(cmd);
    RCLCPP_INFO(get_logger(), "[CommandExecutor::handleCommand] Start command '%s' executed %s", cmd.c_str(), ok ? "successfully" : "failed");
  }
  else
  {
    ok = process_manager_.stopProcess(cmd);
    RCLCPP_INFO(get_logger(), "[CommandExecutor::handleCommand] Stop command '%s' executed %s", cmd.c_str(), ok ? "successfully" : "failed");
  }

  response->success = ok;
}

void CommandExecutor::handleGetMapData(const std::shared_ptr<command_executor_msgs::srv::GetMapData::Request>& request,
                                       std::shared_ptr<command_executor_msgs::srv::GetMapData::Response> response)
{

  const auto& map_path = request->map_path;
  const auto& map_name = request->map_name;

  if (map_path.empty())
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::handleGetMapData] Received empty map path");
    response->success = false;
    response->error_message = "Cannot get map data: map path is empty";
    return;
  }

  if (map_name.empty())
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::handleGetMapData] Received empty map name");
    response->success = false;
    response->error_message = "Cannot get map data: map name is empty";
    return;
  }

  const auto map_yaml_info = ROBOGait::functions::getMapYamlInfo(map_path, map_name);
  if (!map_yaml_info)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::handleGetMapData] Failed to get map YAML info for %s", map_name.c_str());
    response->success = false;
    response->error_message = "Cannot get map YAML info for map: " + map_name;
    return;
  }

  const auto map_pgm_info = ROBOGait::functions::getMapPgmInfo(map_path, map_name);
  if (!map_pgm_info)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::handleGetMapData] Failed to get map PGM info for %s", map_name.c_str());
    response->success = false;
    response->error_message = "Cannot get map PGM info for map: " + map_name;
    return;
  }

  response->yaml_content = *map_yaml_info;
  response->pgm_content = *map_pgm_info;
  response->success = true;
  response->error_message = "";

  RCLCPP_INFO(get_logger(), "[CommandExecutor::handleGetMapData] Successfully retrieved map data for %s", map_name.c_str());
}

void CommandExecutor::mainLoop()
{
  command_executor_msgs::msg::RobotStatus msg;
  msg.id = robot_info_.id;
  msg.battery = robot_info_.battery;
  msg.ns = robot_info_.ns;
  msg.version = robot_info_.version;
  msg.hardware_id = robot_info_.hardware_id;
  msg.serial_number = robot_info_.serial_number;
  pub_robot_status_->publish(msg);
}

bool CommandExecutor::isAllowedCommand(const std::string& cmd) const
{
  const std::string trimmed = ltrimCopy(cmd);
  std::istringstream iss(trimmed);
  std::string first;
  std::string second;
  if (!(iss >> first))
  {
    return false;
  }
  if (first != "ros2")
  {
    return false;
  }
  if (!(iss >> second))
  {
    return false;
  }
  return std::find(allow_list_.begin(), allow_list_.end(), second) != allow_list_.end();
}

std::string CommandExecutor::ltrimCopy(const std::string& value)
{
  const auto pos = value.find_first_not_of(" \t\r\n");
  if (pos == std::string::npos)
  {
    return "";
  }
  return value.substr(pos);
}

bool CommandExecutor::loadConfig()
{
  auto& loader = ROBOGait::loader::YamlLoader::getInstance();
  const std::string config_path = resolveConfigPath();

  if (!loader.loadConfig(config_path))
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] Failed to load config: %s", config_path.c_str());
    return false;
  }

  allow_list_ = loader.getValue<std::vector<std::string>>("allow_list", {});

  if (allow_list_.empty())
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] allow_list is empty");
    return false;
  }

  const int id_value = loader.getValue<int>("robot_info.id", 0);
  robot_info_.id = static_cast<uint8_t>(std::max(0, id_value));
  robot_info_.battery = 0.0; // TODO: subscribe to robot battery status
  robot_info_.ns = loader.getValue<std::string>("robot_info.namespace", "");

  robot_info_.version = loader.getValue<std::string>("robot_info.version", "");

  const int hardware_id_value = loader.getValue<int>("robot_info.hardware_id", 0);
  robot_info_.hardware_id = static_cast<uint32_t>(std::max(0, hardware_id_value));
  robot_info_.serial_number = loader.getValue<std::string>("robot_info.serial_number", "");

  RCLCPP_INFO(get_logger(), "[CommandExecutor::loadConfig] Config loaded successfully from %s", config_path.c_str());

  return true;
}

std::string CommandExecutor::resolveConfigPath()
{
  const std::filesystem::path share_path = ament_index_cpp::get_package_share_directory("command_executor");
  const std::filesystem::path installed = share_path / "params" / "config.yaml";
  return installed.string();
}

bool CommandExecutor::createRosInterfaces()
{
  srv_cmd_ =
      create_service<command_executor_msgs::srv::Cmd>(S_CMD, std::bind(&CommandExecutor::handleCommand, this, std::placeholders::_1, std::placeholders::_2));

  srv_get_map_data_ = create_service<command_executor_msgs::srv::GetMapData>(
      S_GET_MAP_DATA, std::bind(&CommandExecutor::handleGetMapData, this, std::placeholders::_1, std::placeholders::_2));

  pub_robot_status_ = create_publisher<command_executor_msgs::msg::RobotStatus>(T_ROBOT_STATUS, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT());

  timer_ = create_wall_timer(std::chrono::milliseconds(TIME_MAIN_LOOP), std::bind(&CommandExecutor::mainLoop, this)); // one-shot: false, autostart: true

  if (!srv_cmd_)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::createRosInterfaces] Failed to create service");
    return false;
  }
  if (!srv_get_map_data_)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::createRosInterfaces] Failed to create get map data service");
    return false;
  }
  if (!pub_robot_status_)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::createRosInterfaces] Failed to create publisher");
    return false;
  }
  if (!timer_)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::createRosInterfaces] Failed to create timer");
    return false;
  }

  return true;
}

bool CommandExecutor::isDeleteCommand(const std::string& cmd, std::string& translated_cmd) const
{
  const std::string trimmed = ltrimCopy(cmd);
  std::istringstream iss(trimmed);
  std::string first;
  std::string second;

  if (!(iss >> first >> second))
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::isDeleteCommand] Failed to parse command: %s", cmd.c_str());
    return false;
  }

  if (first != "ros2" || second != "delete")
  {
    return false;
  }

  std::string path;
  std::getline(iss, path);
  path = ltrimCopy(path);

  if (path.empty())
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::isDeleteCommand] Delete command missing path: %s", cmd.c_str());
    return false;
  }

  if (path == "/" || path == "/*" || path.find("..") != std::string::npos)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::isDeleteCommand] Unsafe delete path: %s", path.c_str());
    return false;
  }

  translated_cmd = "rm -f " + path;

  return true;
}