#pragma once

#include <string>
#include <vector>

#include <boost/process/child.hpp>
#include <boost/process/group.hpp>

#include "ProcessInterface.hpp"
#include "common/enum.hpp"
namespace ROBOGait
{
namespace process
{
/**
 * @brief Class representing a process.
 */
class Process : public ProcessInterface
{
public:
  /**
   * @brief Constructor for the Process class.
   * @param name Name of the process.
   * @param command Command to execute the process.
   * @param args Arguments for the command.
   */
  Process(const std::string& name, const std::string& command, const std::vector<std::string>& args);

  /**
   * @brief Destructor for the Process class.
   */
  ~Process();

  /**
   * @brief Gets the command of the process.
   * @return The command string.
   */
  const std::string& getCommand() const;

  /**
   * @brief Gets the arguments of the process.
   * @return The vector of argument strings.
   */
  const std::vector<std::string>& getArguments() const;

  /**
   * @brief Runs the process.
   */
  void run() override;

  /**
   * @brief Stops the process.
   */
  void stop() override;

  /**
   * @brief Checks if the process is running.
   * @return True if the process is running, false otherwise.
   */
  bool isRunning();

  /**
   * @brief Gets the PID of the process.
   * @return The process ID.
   */
  boost::process::pid_t getPid() const;

  /**
   * @brief Sets the terminal title for the process.
   * @param title The terminal title string.
   */
  void setTerminalTitle(const std::string& title);
  /**
   * @brief Sets the preamble script to run before the command process.
   * @param preamble The preamble script string.
   */
  void setPreambleScript(const std::string& preamble);

  /**
   * @brief Sets the window state for the terminal.
   * @param state The desired window state.
   */
  void setWindowState(common::WindowState state);

protected:
  /**
   * @brief Builds the shell command string.
   * @return The shell command string.
   */
  std::string buildShellCommand() const;

private:
  std::string command_;                /**< Command to execute the process */
  std::vector<std::string> arguments_; /**< Arguments for the command */
  std::string terminal_title_;         /**< Title for the terminal window */
  std::string preamble_script_;        /**< Script to run before the command process */
  common::WindowState window_state_;   /**< Window state of the terminal */

  // Runtime
  boost::process::group process_group_; /**< Group of processes */
  boost::process::child process_;       /**< The actual process */
};
} // namespace process
} // namespace ROBOGait