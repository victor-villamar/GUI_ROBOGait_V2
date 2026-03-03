#include <vector>

#include <boost/process/args.hpp>
#include <boost/process/search_path.hpp>

#include "ProcessManager.hpp"

using namespace ROBOGait::command;

ProcessManager::~ProcessManager()
{
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& entry : processes_)
  {
    std::error_code ec;
    if (isRunning(entry.second))
    {
      entry.second.group.terminate(ec);
      entry.second.child.wait(ec);
    }
  }
  processes_.clear();
}

bool ProcessManager::startProcess(const std::string& cmd)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto existing = processes_.find(cmd);
  if (existing != processes_.end())
  {
    if (isRunning(existing->second))
    {
      return false;
    }
    processes_.erase(existing);
  }

  auto terminal = boost::process::search_path("xfce4-terminal");
  if (terminal.empty())
  {
    return false;
  }

  const std::string command_arg = std::string("bash -lc \"") + cmd + "\"";
  std::vector<std::string> terminal_args = {
      "--disable-server", "--maximize", "-T", "command_executor", "--command", command_arg,
  };

  boost::process::group group;
  boost::process::child child(terminal.string(), group, boost::process::args(terminal_args));

  const auto pid = child.id();
  ProcessEntry entry{std::move(group), std::move(child), pid};
  processes_.emplace(cmd, std::move(entry));
  return true;
}

bool ProcessManager::stopProcess(const std::string& cmd)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = processes_.find(cmd);
  if (it == processes_.end())
  {
    return false;
  }

  std::error_code ec;
  if (isRunning(it->second))
  {
    it->second.group.terminate(ec);
    it->second.child.wait(ec);
  }

  processes_.erase(it);
  return true;
}

std::optional<boost::process::pid_t> ProcessManager::getPid(const std::string& cmd) const
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = processes_.find(cmd);
  if (it == processes_.end())
  {
    return std::nullopt;
  }
  return it->second.pid;
}

bool ProcessManager::isRunning(ProcessEntry& entry) { return entry.child.valid() && entry.child.running(); }