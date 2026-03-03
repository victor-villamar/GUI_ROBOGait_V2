#pragma once

#include <boost/process/child.hpp>
#include <boost/process/group.hpp>

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace ROBOGait
{
namespace command
{
class ProcessManager
{
public:
  ProcessManager() = default;
  ~ProcessManager();

  bool startProcess(const std::string& cmd);
  bool stopProcess(const std::string& cmd);
  std::optional<boost::process::pid_t> getPid(const std::string& cmd) const;

private:
  /**
   * @brief Struct to hold information about a running process
   *
   * @param group The process group
   * @param child The child process
   * @param pid The process ID
   */
  struct ProcessEntry
  {
    boost::process::group group;
    boost::process::child child;
    boost::process::pid_t pid{-1};
  };

  bool isRunning(ProcessEntry& entry);

  mutable std::mutex mutex_;                                /**< Mutex for synchronizing access to the process map */
  std::unordered_map<std::string, ProcessEntry> processes_; /**< Map of running processes */
};
} // namespace command
} // namespace ROBOGait
