#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <boost/process/args.hpp>
#include <boost/process/search_path.hpp>
#include <logs/Logs.hpp>

#include "process/Process.hpp"

using namespace ROBOGait::process;
using namespace ROBOGait::common;
using namespace ROBOGait::common::logs;

Process::Process(const std::string& name, const std::string& command, const std::vector<std::string>& args) : command_(command), arguments_(args)
{
  setName(name);
  setState(ROBOGait::common::ProcessState::STOPPED);
  terminal_title_ = "";
  preamble_script_ = "";
  window_state_ = WindowState::NOT_DEFINED;
}

Process::~Process()
{
  if (getState() == ProcessState::RUNNING)
  {
    stop();
  }
}

const std::string& Process::getCommand() const { return command_; }

const std::vector<std::string>& Process::getArguments() const { return arguments_; }

void Process::run()
{
  if (getState() == ProcessState::RUNNING)
  {
    Logs::infoStream("[Process::run] Process '" + getName() + "' is already running.");
    return;
  }

  const std::string shell_command = buildShellCommand();
  const std::string title = terminal_title_.empty() ? getName() : terminal_title_;

  auto terminal = boost::process::search_path("xfce4-terminal");

  if (terminal.empty())
  {
    Logs::errorStream("[Process::run] xfce4-terminal not found in PATH. Cannot start process '" + getName() + "'.");
    setState(ProcessState::STOPPED);
    return;
  }

  const std::string command_arg = std::string("bash -lc \"") + shell_command + "\"";
  std::vector<std::string> terminal_args;

  if (window_state_ == WindowState::MAXIMIZED)
  {
    terminal_args = {"--disable-server", "--maximize", "-T", title, "--command", command_arg};
  }
  else if (window_state_ == WindowState::MINIMIZED)
  {
    terminal_args = {"--disable-server", "--minimize", "-T", title, "--command", command_arg};
  }

  std::cout << "========================================== Process " + title + " ==========================================" << std::endl;

  const std::string joined_args = boost::algorithm::join(terminal_args, " ");
  std::cout << "Process: (" + joined_args + ")" << std::endl;

  try
  {
    process_group_ = boost::process::group();

    // clang-format off
    process_ = boost::process::child(
               terminal.string(),
               process_group_,
               boost::process::args(terminal_args));
    // clang-format on

    setState(ProcessState::RUNNING);

    const auto pid = process_.id();

    std::cout << "Process PID: '" << pid << "'" << std::endl;
  }
  catch (const std::exception& e)
  {
    Logs::errorStream("[Process::run] Failed to start process '" + command_ + "'. Exception: " + std::string(e.what()));
    setState(ProcessState::UNKNOWN);
  }
}

void Process::stop()
{
  if (getState() == ProcessState::STOPPED)
  {
    Logs::infoStream("[Process::stop] Process '" + getName() + "' is already stopped.");
    return;
  }

  std::error_code ec;

  try
  {
    if (isRunning())
    {
      process_group_.terminate(ec);
      process_.wait(ec);
    }
    process_ = boost::process::child();
    process_group_ = boost::process::group();
    setState(ProcessState::STOPPED);
  }
  catch (const std::exception& e)
  {
    Logs::errorStream("[Process::stop] Failed to stop process '" + command_ + "'. Exception: " + std::string(e.what()));
    setState(ProcessState::UNKNOWN);
  }
}

boost::process::pid_t Process::getPid() const { return process_.valid() > 0 ? process_.id() : static_cast<boost::process::pid_t>(-1); }

bool Process::isRunning() { return process_.valid() && process_.running(); }

void Process::setTerminalTitle(const std::string& title) { terminal_title_ = title; }

void Process::setPreambleScript(const std::string& preamble) { preamble_script_ = preamble; }

void Process::setWindowState(common::WindowState state) { window_state_ = state; }

std::string Process::buildShellCommand() const
{
  std::string cmd_line;

  if (!preamble_script_.empty())
  {
    cmd_line += preamble_script_ + "; ";
  }

  cmd_line += command_;

  if (!arguments_.empty())
  {
    cmd_line.push_back(' ');
    cmd_line += boost::algorithm::join(arguments_, " ");
  }

  return cmd_line;
}
