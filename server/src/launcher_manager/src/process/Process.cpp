#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <boost/process/args.hpp>
#include <boost/process/io.hpp>
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

  std::string executable_path;
  std::vector<std::string> process_args;
  std::string display_command;

#ifdef TERMINAL
  const auto terminal = boost::process::search_path("xfce4-terminal");

  if (terminal.empty())
  {
    Logs::errorStream("[Process::run] xfce4-terminal not found in PATH. Cannot start process '" + getName() + "'.");
    setState(ProcessState::STOPPED);
    return;
  }

  executable_path = terminal.string();

  const std::string command_arg = std::string("bash -lc \"") + shell_command + "\"";

  if (window_state_ == WindowState::MAXIMIZED)
  {
    process_args = {"--disable-server", "--maximize", "-T", title, "--command", command_arg};
  }
  else if (window_state_ == WindowState::MINIMIZED)
  {
    process_args = {"--disable-server", "--minimize", "-T", title, "--command", command_arg};
  }

  display_command = executable_path + " " + boost::algorithm::join(process_args, " ");

#else
  const auto bash = boost::process::search_path("bash");

  if (bash.empty())
  {
    Logs::errorStream("[Process::run] bash not found in PATH. Cannot start process '" + getName() + "'.");
    setState(ProcessState::STOPPED);
    return;
  }

  executable_path = bash.string();
  process_args = {"-lc", shell_command};
  display_command = executable_path + " " + boost::algorithm::join(process_args, " ");

#endif

  std::cout << "========================================== Process " + title + " ==========================================" << std::endl;
  std::cout << "Process: (" + display_command + ")" << std::endl;

  try
  {
    process_group_ = boost::process::group();
#ifdef TERMINAL
    // clang-format off
    process_ = boost::process::child(
               executable_path,
               process_group_,
               boost::process::args(process_args));
    // clang-format on
#else
    // clang-format off
    process_ = boost::process::child(
               executable_path,
               process_group_,
               boost::process::args(process_args),
               boost::process::std_in < boost::process::null,
               boost::process::std_out > boost::process::null,
               boost::process::std_err > boost::process::null);
    // clang-format on
#endif
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
