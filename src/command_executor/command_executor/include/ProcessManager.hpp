#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <boost/version.hpp>

#if BOOST_VERSION >= 108800
#include <boost/process/v1/args.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/group.hpp>
#include <boost/process/v1/io.hpp>
#else
#include <boost/process/args.hpp>
#include <boost/process/child.hpp>
#include <boost/process/group.hpp>
#include <boost/process/io.hpp>
#endif
namespace ROBOGait
{
namespace command
{

#if BOOST_VERSION >= 108800
namespace boost_process = boost::process::v1;
#else
namespace boost_process = boost::process;
#endif

/**
 * @brief ProcessManager class for managing system processes
 */
class ProcessManager
{
public:
  /**
   * @brief Constructor for the ProcessManager class
   */
  ProcessManager();

  /**
   * @brief Destructor for the ProcessManager class
   */
  ~ProcessManager();

  /**
   * @brief Start a new process
   *
   * @param cmd The command to execute
   *
   * @return true if the process was started successfully, false otherwise
   */
  bool startProcess(const std::string& cmd);

  /**
   * @brief Stop a running process
   *
   * @param cmd The command of the process to stop
   *
   * @return true if the process was stopped successfully, false otherwise`
   */
  bool stopProcess(const std::string& cmd);

  /**
   * @brief Execute a one-shot command
   *
   * @param cmd The command to execute
   *
   * @return true if the command was executed successfully, false otherwise
   */
  bool executeOneShotCommand(const std::string& cmd);

  /**
   *@brief Set the ROS namespace for the command executor node
   *
   *@param robot_namespace ROS namespace of the command executor node
   */
  void setRobotNamespace(const std::string& robot_namespace);

private:
  /**
   * @brief Struct to hold information about a running process
   *
   * @param group The process group
   * @param child The child process
   * @param pid The process ID
   * @param is_stopping Whether a stop operation is in progress
   */
  struct ProcessEntry
  {
    boost_process::group group;
    boost_process::child child;
    boost_process::pid_t pid{-1};
    bool is_stopping{false};
  };

  /**
   * @brief Check if a process is running
   *
   * @param entry The process entry to check
   *
   * @return true if the process is running, false otherwise
   */
  bool isRunning(ProcessEntry& entry);

  mutable std::mutex mutex_;                                                 /**< Mutex for synchronizing access to the process map */
  std::unordered_map<std::string, std::shared_ptr<ProcessEntry>> processes_; /**< Map of running processes */
  std::string robot_namespace_;                                              /**< Robot namespace */
};
} // namespace command
} // namespace ROBOGait