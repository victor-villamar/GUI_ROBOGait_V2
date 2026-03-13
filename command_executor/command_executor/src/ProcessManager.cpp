#include <iostream>
#include <signal.h>
#include <vector>

#include <boost/process/args.hpp>
#include <boost/process/io.hpp>
#include <boost/process/search_path.hpp>

#include "ProcessManager.hpp"

using namespace ROBOGait::command;

ProcessManager::ProcessManager() { std::cout << "[ProcessManager::ProcessManager] ProcessManager initialized" << std::endl; }
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

  std::cout << "[ProcessManager::~ProcessManager] ProcessManager destroyed" << std::endl;
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

  boost::process::group group;

  // clang-format off
  boost::process::child child(
    "/bin/bash",
    boost::process::args({"-lc", cmd}),
    group,
    boost::process::std_out > "/tmp/command_executor_stdout.log",
    boost::process::std_err > "/tmp/command_executor_stderr.log"
  );
  // clang-format on

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
    const pid_t group_pid = it->second.pid;

    ::kill(-group_pid, SIGINT);

    if (it->second.child.wait_for(std::chrono::seconds(1), ec))
    {
      processes_.erase(it);
      return true;
    }

    std::cout << "[ProcessManager::stopProcess] Process not terminated gracefully, forcing kill" << std::endl;
    ::kill(-group_pid, SIGKILL);
    it->second.child.wait_for(std::chrono::milliseconds(50), ec);
  }

  processes_.erase(it);
  return true;
}

bool ProcessManager::executeOneShotCommand(const std::string& cmd)
{
  std::lock_guard<std::mutex> lock(mutex_);

  // clang-format off
  boost::process::child child(
    "/bin/bash",
    boost::process::args({"-lc", cmd}),
    boost::process::std_out > "/tmp/command_executor_stdout.log",
    boost::process::std_err > "/tmp/command_executor_stderr.log"
  );
  // clang-format on

  child.wait();

  const int exit_code = child.exit_code();

  if (exit_code != 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool ProcessManager::isRunning(ProcessEntry& entry) { return entry.child.valid() && entry.child.running(); }