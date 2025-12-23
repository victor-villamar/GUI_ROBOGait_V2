#include <chrono>
#include <csignal>
#include <iostream>
#include <logs/Logs.hpp>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include "process/Process.hpp"

using namespace ROBOGait::process;
using namespace ROBOGait::common;
using namespace ROBOGait::common::logs;

// Global variables for access from the signal handler
std::vector<std::unique_ptr<Process>> g_processes;

// Function to clean up all processes
void cleanupAllProcesses()
{
  std::cout << "\n[CLEANUP] Stopping all processes..." << std::endl;

  for (auto& process : g_processes)
  {
    if (process && process->getState() == ProcessState::RUNNING)
    {
      std::cout << "[CLEANUP] Stopping process '" << process->getName() << "'..." << std::endl;
      process->stop();
    }
  }

  std::cout << "[CLEANUP] All processes have been stopped." << std::endl;
}

// Signal handler for Ctrl+C
void signalHandler(int signal)
{
  if (signal == SIGINT)
  {
    std::cout << "\n[SIGNAL] Ctrl+C detected. Cleaning up processes..." << std::endl;
    cleanupAllProcesses();
    std::cout << "[SIGNAL] Exiting program." << std::endl;
    exit(0);
  }
}

void showProcessStatus(const std::unique_ptr<Process>& process)
{
  if (!process)
  {
    std::cout << "  Process: NULL" << std::endl;
    return;
  }

  std::string stateStr;
  switch (process->getState())
  {
    case ProcessState::RUNNING:
      stateStr = "RUNNING";
      break;
    case ProcessState::STOPPED:
      stateStr = "STOPPED";
      break;
    case ProcessState::UNKNOWN:
      stateStr = "UNKNOWN";
      break;
    default:
      stateStr = "UNDEFINED";
      break;
  }

  std::cout << "  " << process->getName() << " (State: " << stateStr;

  if (process->getState() == ProcessState::RUNNING)
  {
    std::cout << ", PID: " << process->getPid();
  }

  std::cout << ")" << std::endl;
}

int main()
{

  // Set up the signal handler
  signal(SIGINT, signalHandler);

  Logs::info("[main] Starting ROBOGait Process Test");

  // Create test processes with different configurations

  // Process 1: Simple echo command
  auto process1 =
      std::make_unique<Process>("echo_test", "echo", std::vector<std::string>{"'Hello from Process 1!'", "&&", "read", "-p", "'Press Enter to close...'"});
  process1->setTerminalTitle("Echo Test Process");
  process1->setWindowState(WindowState::MAXIMIZED);

  // Process 2: Log test from logs package
  auto process2 = std::make_unique<Process>("logs_test", "ros2", std::vector<std::string>{"run", "logs", "test_logs"});
  process2->setTerminalTitle("Logs Test Process");
  process2->setWindowState(WindowState::MAXIMIZED);

  // Process 3: Long running process with preamble
  auto process3 = std::make_unique<Process>("watch_test", "watch", std::vector<std::string>{"-n", "2", "date"});
  process3->setTerminalTitle("Watch Date Process");
  process3->setPreambleScript("echo 'Starting watch process...'; sleep 1");
  process3->setWindowState(WindowState::MINIMIZED);

  // Process 4: Interactive shell
  auto process4 = std::make_unique<Process>("shell_test", "bash", std::vector<std::string>{"-i"});
  process4->setTerminalTitle("Interactive Shell");
  process4->setWindowState(WindowState::MAXIMIZED);

  // Process 5: Python script simulation
  auto process5 = std::make_unique<Process>(
      "python_test", "python3",
      std::vector<std::string>{
          "-c", "\'import time; [print(f\\\"Python process tick {i}\\\") or time.sleep(2) for i in range(5)]; input(\\\"Press Enter to close...\\\")\'"});
  process5->setTerminalTitle("Python Test Process");
  process5->setWindowState(WindowState::MINIMIZED);

  // Store all processes in global vector for cleanup
  g_processes.push_back(std::move(process1));
  g_processes.push_back(std::move(process2));
  g_processes.push_back(std::move(process3));
  g_processes.push_back(std::move(process4));
  g_processes.push_back(std::move(process5));

  std::cout << "=== ROBOGait Process Manager Test ===" << std::endl;
  std::cout << "Available processes:" << std::endl;
  std::cout << "  1. echo_test      -> Simple echo command" << std::endl;
  std::cout << "  2. logs_test      -> ROS 2 logs test (Maximized window)" << std::endl;
  std::cout << "  3. watch_test     -> Watch date command with preamble (Minimized window)" << std::endl;
  std::cout << "  4. shell_test     -> Interactive bash shell" << std::endl;
  std::cout << "  5. python_test    -> Python countdown script" << std::endl;
  std::cout << std::endl;
  std::cout << "Commands:" << std::endl;
  std::cout << "  run <id>     -> Start process by ID (1-5)" << std::endl;
  std::cout << "  stop <id>    -> Stop process by ID (1-5)" << std::endl;
  std::cout << "  status       -> Check status of all processes" << std::endl;
  std::cout << "  info <id>    -> Show detailed info of process" << std::endl;
  std::cout << "  runall       -> Start all processes" << std::endl;
  std::cout << "  stopall      -> Stop all processes" << std::endl;
  std::cout << "  test         -> Run automated test sequence" << std::endl;
  std::cout << "  quit         -> Exit program" << std::endl;
  std::cout << "  Ctrl+C       -> Emergency stop all and exit" << std::endl;
  std::cout << std::endl;
  std::cout << "NOTE: Processes will open in separate terminal windows." << std::endl;
  std::cout << "      Use the stop command to properly terminate them." << std::endl;
  std::cout << std::endl;

  std::string input;
  while (true)
  {
    std::cout << "process_test> ";
    std::getline(std::cin, input);

    if (input == "quit" || input == "exit")
    {
      std::cout << "[QUIT] Quit command detected. Cleaning up processes..." << std::endl;
      cleanupAllProcesses();
      std::cout << "[QUIT] Test finished." << std::endl;
      break;
    }

    if (input == "status")
    {
      std::cout << "Status of all processes:" << std::endl;
      for (const auto& process : g_processes)
      {
        showProcessStatus(process);
      }
      continue;
    }

    if (input == "runall")
    {
      std::cout << "Starting all processes..." << std::endl;
      for (auto& process : g_processes)
      {
        if (process->getState() != ProcessState::RUNNING)
        {
          std::cout << "Starting process '" << process->getName() << "'..." << std::endl;
          process->run();
          std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Small delay between starts
        }
      }
      continue;
    }

    if (input == "stopall")
    {
      std::cout << "Stopping all processes..." << std::endl;
      cleanupAllProcesses();
      continue;
    }

    if (input == "test")
    {
      std::cout << "=== Running Automated Test Sequence ===" << std::endl;

      // Test 1: Start and stop echo process
      std::cout << "\n[TEST 1] Testing echo process..." << std::endl;
      g_processes[0]->run();
      std::this_thread::sleep_for(std::chrono::seconds(3));
      g_processes[0]->stop();
      std::cout << "[TEST 1] Echo process test completed." << std::endl;

      // Test 2: Start logs test with maximized window
      std::cout << "\n[TEST 2] Testing logs process with maximized window..." << std::endl;
      g_processes[1]->run();
      std::this_thread::sleep_for(std::chrono::seconds(5));
      g_processes[1]->stop();
      std::cout << "[TEST 2] Logs process test completed." << std::endl;

      // Test 3: Start watch with preamble and minimized window
      std::cout << "\n[TEST 3] Testing watch process with preamble and minimized window..." << std::endl;
      g_processes[2]->run();
      std::this_thread::sleep_for(std::chrono::seconds(5));
      g_processes[2]->stop();
      std::cout << "[TEST 3] Watch process test completed." << std::endl;

      // Test 4: Multiple processes running simultaneously
      std::cout << "\n[TEST 4] Testing multiple processes simultaneously..." << std::endl;
      g_processes[0]->run(); // echo
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      g_processes[4]->run(); // python
      std::this_thread::sleep_for(std::chrono::seconds(3));
      g_processes[0]->stop();
      g_processes[4]->stop();
      std::cout << "[TEST 4] Multiple processes test completed." << std::endl;

      std::cout << "\n=== Automated Test Sequence Completed ===" << std::endl;
      continue;
    }

    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
    {
      tokens.push_back(token);
    }

    if (tokens.empty())
      continue;

    if (tokens.size() == 2)
    {
      std::string command = tokens[0];
      int processId = 0;

      try
      {
        processId = std::stoi(tokens[1]);
      }
      catch (const std::exception&)
      {
        std::cout << "Invalid process ID. Use numbers 1-5." << std::endl;
        continue;
      }

      if (processId < 1 || processId > 5)
      {
        std::cout << "Process ID out of range. Use numbers 1-5." << std::endl;
        continue;
      }

      auto& process = g_processes[processId - 1];

      if (command == "run")
      {
        if (process->getState() == ProcessState::RUNNING)
        {
          std::cout << "Process '" << process->getName() << "' is already running." << std::endl;
        }
        else
        {
          std::cout << "Starting process '" << process->getName() << "'..." << std::endl;
          process->run();
        }
      }
      else if (command == "stop")
      {
        if (process->getState() == ProcessState::STOPPED)
        {
          std::cout << "Process '" << process->getName() << "' is already stopped." << std::endl;
        }
        else
        {
          std::cout << "Stopping process '" << process->getName() << "'..." << std::endl;
          process->stop();
        }
      }
      else if (command == "info")
      {
        std::cout << "Process Information:" << std::endl;
        std::cout << "  Name: " << process->getName() << std::endl;
        std::cout << "  Command: " << process->getCommand() << std::endl;

        const auto& args = process->getArguments();
        if (!args.empty())
        {
          std::cout << "  Arguments: ";
          for (size_t i = 0; i < args.size(); ++i)
          {
            std::cout << args[i];
            if (i < args.size() - 1)
              std::cout << " ";
          }
          std::cout << std::endl;
        }

        std::string stateStr;
        switch (process->getState())
        {
          case ProcessState::RUNNING:
            stateStr = "RUNNING";
            break;
          case ProcessState::STOPPED:
            stateStr = "STOPPED";
            break;
          case ProcessState::UNKNOWN:
            stateStr = "UNKNOWN";
            break;
          default:
            stateStr = "UNDEFINED";
            break;
        }
        std::cout << "  State: " << stateStr << std::endl;

        if (process->getState() == ProcessState::RUNNING)
        {
          std::cout << "  PID: " << process->getPid() << std::endl;
          std::cout << "  Is Running: " << (process->isRunning() ? "Yes" : "No") << std::endl;
        }
      }
      else
      {
        std::cout << "Unknown command: " << command << std::endl;
      }
    }
    else
    {
      std::cout << "Unrecognized command format. Use: <command> <id>" << std::endl;
      std::cout << "Type 'quit' to exit or use commands without parameters like 'status', 'runall', 'stopall', 'test'." << std::endl;
    }
  }

  // Final cleanup
  cleanupAllProcesses();

  Logs::info("[main] ROBOGait Process Test finished");

  std::cout << "Test finished." << std::endl;
  return 0;
}
