#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/enum.hpp"
#include "process/Process.hpp"

namespace ROBOGait
{
namespace controllers
{
/**
 * @brief Class for controlling multiple processes.
 */
class ProcessController
{
public:
  /**
   * @brief Constructor for the ProcessController class.
   */
  ProcessController();

  /**
   * @brief Destructor for the ProcessController class.
   */
  ~ProcessController();

  /**
   * @brief Checks if a ROS2 node is running.
   * @param node_name Name of the ROS2 node.
   * @return True if the node is running, false otherwise.
   */
  bool isNodeRunning(const std::string& node_name);

  /**
   * @brief Starts a new process.
   * @param name Name of the process.
   * @param executable Command to execute the process.
   * @param arguments Arguments for the command.
   * @param window_state Desired window state for the terminal.
   */
  void startProcess(const std::string& name, const std::string& executable, const std::vector<std::string>& arguments,
                    common::WindowState window_state = common::WindowState::MINIMIZED);

  /**
   * @brief Stops a running process.
   * @param name Name of the process to stop.
   */
  void stopProcess(const std::string& name);

  /**
   * @brief Stops all running processes.
   */
  void stopAllProcesses();

  /**
   * @brief Lists all running processes.
   */
  void listProcesses() const;

  /**
   * @brief Returns a string representation of all running processes.
   * @return String containing the names of all running processes.
   */
  std::string listProcessesString() const;

private:
  std::unordered_map<std::string, std::unique_ptr<ROBOGait::process::Process>> processMap; /**< Map of process names */
};
} // namespace controllers
} // namespace ROBOGait
