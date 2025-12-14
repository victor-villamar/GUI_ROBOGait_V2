#include <iostream>
#include <rclcpp/node.hpp>

#include "../include/ProcessController.hpp"

#include "logs/Logs.hpp"

using namespace ROBOGait::controllers;
using namespace ROBOGait::common;
using namespace ROBOGait::common::logs;

ProcessController::ProcessController() {}

ProcessController::~ProcessController() { stopAllProcesses(); }

bool ProcessController::isNodeRunning(const std::string& node_name)
{
  auto node = rclcpp::Node::make_shared("node_checker");
  auto node_names = node->get_node_names();

  for (const auto& name : node_names)
  {
    if (name == node_name)
    {
      return true;
    }
  }
  return false;
}

void ProcessController::startProcess(const std::string& name, const std::string& executable, const std::vector<std::string>& arguments,
                                     WindowState window_state)
{
  // Verify if it's a ROS2 node and if it's already running
  if (executable == "ros2" && !arguments.empty() && arguments.front() == "run" && isNodeRunning(name))
  {
    Logs::infoStream("[ProcessController::startProcess] The ROS2 node '" + name + "' is already running.");
    return;
  }

  // Check if the process is already running
  if (processMap.find(name) != processMap.end())
  {
    Logs::infoStream("[ProcessController::startProcess] The process '" + name + "' is already running.");
    return;
  }

  if (executable.empty())
  {
    Logs::errorStream("[ProcessController::startProcess] Empty executable for process '" + name + "'.");
    return;
  }

  auto process = std::make_unique<ROBOGait::process::Process>(name, executable, arguments);
  process->setTerminalTitle(name);
  process->setWindowState(window_state);
  process->run();

  if (!process->isRunning())
  {
    Logs::errorStream("[ProcessController::startProcess] Failed to start process '" + name + "'.");
    return;
  }

  auto it = processMap.emplace(name, std::move(process)).first;
  Logs::infoStream("[ProcessController::startProcess] Started process '" + name + "' with PID: " + std::to_string(it->second->getPid()));
}

void ProcessController::stopProcess(const std::string& name)
{
  auto it = processMap.find(name);
  if (it != processMap.end())
  {
    it->second->stop();
    Logs::infoStream("[ProcessController::stopProcess] Stopped process '" + name + "'.");
    processMap.erase(it);
  }
  else
  {
    Logs::errorStream("[ProcessController::stopProcess] Process '" + name + "' is not running.");
  }
}

void ProcessController::stopAllProcesses()
{
  for (auto& entry : processMap)
  {
    Logs::infoStream("[ProcessController::stopAllProcesses] Stopping process '" + entry.first + "'.");
    entry.second->stop();
  }
  processMap.clear();
  Logs::infoStream("[ProcessController::stopAllProcesses] All processes have been stopped.");
}

void ProcessController::listProcesses() const
{
  if (processMap.empty())
  {
    Logs::infoStream("[ProcessController::listProcesses] No processes are running.");
  }
  else
  {
    Logs::infoStream("[ProcessController::listProcesses] Processes in execution:");
    for (const auto& [name, process] : processMap)
    {
      std::cout << "  - " << name << " (PID: " << process->getPid() << ")\n";
    }
  }
}

std::string ProcessController::listProcessesString() const
{
  if (processMap.empty())
  {
    Logs::infoStream("[ProcessController::listProcessesString] No processes are running.");
  }
  else
  {
    std::string result;
    Logs::infoStream("[ProcessController::listProcessesString] Running processes:");
    for (const auto& [name, process] : processMap)
    {
      result += name + " ";
      std::cout << "  - " << name << " (PID: " << process->getPid() << ")\n";
    }
    return result;
  }
  return "";
}
