#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <sstream>

#include <rclcpp/create_publisher.hpp>
#include <rclcpp/create_service.hpp>
#include <rclcpp/create_subscription.hpp>
#include <rclcpp/create_timer.hpp>

#include "CommandExecutor.hpp"
#include "Define.hpp"
#include "Functions.hpp"
#include "QoSProfile.hpp"

using namespace ROBOGait::command;

CommandExecutor::CommandExecutor(const rclcpp::NodeOptions& options) : Node(define::COMMAND_EXECUTOR, options)
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

bool CommandExecutor::loadConfig()
{
  try
  {
    allow_list_ = declareOrGetParameter<std::vector<std::string>>("allow_list");

    if (allow_list_.empty())
    {
      RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] allow_list is empty");
      return false;
    }

    const int64_t id_value = declareOrGetParameter<int64_t>("robot_info.id");
    if (id_value < 0 || id_value > std::numeric_limits<uint8_t>::max())
    {
      RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] robot_info.id is out of range: %ld", id_value);
      return false;
    }
    robot_info_.id = static_cast<uint8_t>(id_value);

    robot_info_.ns = declareOrGetParameter<std::string>("robot_info.namespace");
    if (robot_info_.ns.empty())
    {
      RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] robot_info.namespace is empty");
      return false;
    }

    robot_info_.version = declareOrGetParameter<std::string>("robot_info.version");
    if (robot_info_.version.empty())
    {
      RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] robot_info.version is empty");
      return false;
    }

    const int64_t hardware_id_value = declareOrGetParameter<int64_t>("robot_info.hardware_id");
    if (hardware_id_value < 0 || hardware_id_value > std::numeric_limits<uint32_t>::max())
    {
      RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] robot_info.hardware_id is out of range: %ld", hardware_id_value);
      return false;
    }
    robot_info_.hardware_id = static_cast<uint32_t>(hardware_id_value);

    robot_info_.serial_number = declareOrGetParameter<std::string>("robot_info.serial_number");
    if (robot_info_.serial_number.empty())
    {
      RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] robot_info.serial_number is empty");
      return false;
    }
  }
  catch (const std::exception& e)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::loadConfig] %s", e.what());
    return false;
  }

  RCLCPP_INFO(get_logger(), "[CommandExecutor::loadConfig] Config loaded successfully");

  return true;
}

bool CommandExecutor::createRosInterfaces()
{
  srv_cmd_ = create_service<command_executor_msgs::srv::Cmd>(define::S_CMD,
                                                             std::bind(&CommandExecutor::handleCommand, this, std::placeholders::_1, std::placeholders::_2));

  srv_get_map_data_ = create_service<command_executor_msgs::srv::GetMapData>(
      define::S_GET_MAP_DATA, std::bind(&CommandExecutor::handleGetMapData, this, std::placeholders::_1, std::placeholders::_2));

  pub_robot_status_ = create_publisher<command_executor_msgs::msg::RobotStatus>(define::T_ROBOT_STATUS, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT());

  sub_battery_status_ = create_subscription<sensor_msgs::msg::BatteryState>(define::T_BATTERY_STATUS, ROBOGait::ros::QosProfiles::QOS_BEST_EFFORT(),
                                                                            std::bind(&CommandExecutor::callbackBatteryStatus, this, std::placeholders::_1));

  timer_ = create_wall_timer(define::TIME_MAIN_LOOP_MS, std::bind(&CommandExecutor::mainLoop, this)); // one-shot: false, autostart: true

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

  if (!sub_battery_status_)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::createRosInterfaces] Failed to create battery status subscriber");
    return false;
  }

  if (!timer_)
  {
    RCLCPP_ERROR(get_logger(), "[CommandExecutor::createRosInterfaces] Failed to create timer");
    return false;
  }

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

void CommandExecutor::callbackBatteryStatus(const sensor_msgs::msg::BatteryState::ConstSharedPtr msg) { robot_info_.battery = msg->percentage; }

void CommandExecutor::mainLoop()
{
  command_executor_msgs::msg::RobotStatus msg;
  msg.id = robot_info_.id;
  msg.battery = robot_info_.battery;
  msg.ns = robot_info_.ns;
  msg.version = robot_info_.version;
  msg.hardware_id = robot_info_.hardware_id;
  msg.serial_number = robot_info_.serial_number;

  if (pub_robot_status_)
  {
    pub_robot_status_->publish(std::move(msg));
  }
}