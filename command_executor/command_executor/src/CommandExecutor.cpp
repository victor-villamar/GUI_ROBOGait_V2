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

using namespace ROBOGait::command;

CommandExecutor::CommandExecutor() : Node(CMD_EXECUTOR)
{
  loadConfig();

  srv_cmd_ =
      create_service<command_executor_msgs::srv::Cmd>(S_CMD, std::bind(&CommandExecutor::handleCommand, this, std::placeholders::_1, std::placeholders::_2));

  pub_robot_status_ = create_publisher<command_executor_msgs::msg::RobotStatus>(T_ROBOT_STATUS, QOS_RELIABLE);

  timer_ = create_wall_timer(std::chrono::milliseconds(TIME_MAIN_LOOP), std::bind(&CommandExecutor::mainLoop, this)); // one-shot: false, autostart: true

  RCLCPP_INFO(get_logger(), "CommandExecutor ready");
}

void CommandExecutor::handleCommand(const std::shared_ptr<command_executor_msgs::srv::Cmd::Request>& request,
                                    std::shared_ptr<command_executor_msgs::srv::Cmd::Response> response)
{
  const auto& cmd = request->cmd;

  if (!isAllowedCommand(cmd))
  {
    RCLCPP_WARN(get_logger(), "Rejected command '%s', not allowed", cmd.c_str());
    response->success = false;
    return;
  }

  bool ok = false;

  if (request->execute)
  {
    ok = process_manager_.startProcess(cmd);
    RCLCPP_INFO(get_logger(), "Start command '%s' executed %s", cmd.c_str(), ok ? "successfully" : "failed");
  }
  else
  {
    ok = process_manager_.stopProcess(cmd);
    RCLCPP_INFO(get_logger(), "Stop command '%s' executed %s", cmd.c_str(), ok ? "successfully" : "failed");
  }

  response->success = ok;
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

void CommandExecutor::loadConfig()
{
  auto& loader = ROBOGait::loader::YamlLoader::getInstance();
  const std::string config_path = resolveConfigPath();

  if (!loader.loadConfig(config_path))
  {
    RCLCPP_ERROR(get_logger(), "Failed to load config: %s", config_path.c_str());
  }

  allow_list_ = loader.getValue<std::vector<std::string>>("allow_list", {});

  // Fall back to defaults if allow_list is empty
  if (allow_list_.empty())
  {
    allow_list_ = {"action", "bag", "node", "component", "param", "control", "pkg", "run", "security", "service", "topic", "interface", "launch", "lifecycle"};
    RCLCPP_WARN(get_logger(), "allow_list empty; using defaults (%zu entries).", allow_list_.size());
  }

  const int id_value = loader.getValue<int>("robot_info.id", 0);
  robot_info_.id = static_cast<uint8_t>(std::max(0, id_value));
  robot_info_.battery = loader.getValue<float>("robot_info.battery", 0.0f);
  robot_info_.ns = loader.getValue<std::string>("robot_info.namespace", "");

  const std::string version_str = loader.getValue<std::string>("robot_info.version", "");
  if (!version_str.empty())
  {
    robot_info_.version = version_str;
  }
  else
  {
    const int version_value = loader.getValue<int>("robot_info.version", 0);
    robot_info_.version = std::to_string(version_value);
  }

  const int hardware_id_value = loader.getValue<int>("robot_info.hardware_id", 0);
  robot_info_.hardware_id = static_cast<uint32_t>(std::max(0, hardware_id_value));
  robot_info_.serial_number = loader.getValue<std::string>("robot_info.serial_number", "");
}

std::string CommandExecutor::resolveConfigPath()
{
  const std::filesystem::path share_path = ament_index_cpp::get_package_share_directory("command_executor");
  const std::filesystem::path installed = share_path / "params" / "config.yaml";
  return installed.string();
}