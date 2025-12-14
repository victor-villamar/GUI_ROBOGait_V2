#include <chrono>
#include <csignal>
#include <iostream>
#include <rclcpp/utilities.hpp>
#include <sstream>
#include <thread>

#include "../include/ProcessController.hpp"
#include "logs/Logs.hpp"

using namespace ROBOGait::controllers;
using namespace ROBOGait::common;
using namespace ROBOGait::common::logs;

// Global ProcessController instance
ProcessController g_processController;

// Test process configurations
struct TestProcess
{
  std::string name;
  std::string executable;
  std::vector<std::string> arguments;
  WindowState window_state;
  std::string description;
};

std::vector<TestProcess> g_testProcesses = {
    {"echo_test",
     "echo",
     {"'Hello from ProcessController Test!'", "&&", "read", "-p", "'Press Enter to close...'"},
     WindowState::MAXIMIZED,
     "Simple echo command (Maximized)"},

    {"logs_test", "ros2", {"run", "logs", "test_logs"}, WindowState::MAXIMIZED, "ROS 2 logs test (Maximized)"},

    {"watch_test", "watch", {"-n", "2", "date"}, WindowState::MINIMIZED, "Watch date command (Minimized)"},

    {"shell_test", "bash", {"-i"}, WindowState::MAXIMIZED, "Interactive bash shell (Maximized)"},

    {"python_test",
     "python3",
     {"-c", "\'import time; [print(f\\\"Python process tick {i}\\\") or time.sleep(2) for i in range(5)]; input(\\\"Press Enter to close...\\\")\'"},
     WindowState::MINIMIZED,
     "Python countdown script (Minimized)"}};

// Signal handler for Ctrl+C
void signalHandler(int signal)
{
  if (signal == SIGINT)
  {
    std::cout << "\n[SIGNAL] Ctrl+C detected. Cleaning up all processes..." << std::endl;
    g_processController.stopAllProcesses();
    std::cout << "[SIGNAL] Exiting program." << std::endl;
    exit(0);
  }
}

void showHelp()
{
  std::cout << "=== ROBOGait ProcessController Test ===" << std::endl;
  std::cout << "Available test processes:" << std::endl;
  for (size_t i = 0; i < g_testProcesses.size(); ++i)
  {
    std::cout << "  " << (i + 1) << ". " << g_testProcesses[i].name << " -> " << g_testProcesses[i].description << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Commands:" << std::endl;
  std::cout << "  start <id>       -> Start process by ID (1-" << g_testProcesses.size() << ")" << std::endl;
  std::cout << "  start <name>     -> Start process by name" << std::endl;
  std::cout << "  stop <name>      -> Stop specific process by name" << std::endl;
  std::cout << "  stopall          -> Stop all processes" << std::endl;
  std::cout << "  list             -> List all running processes" << std::endl;
  std::cout << "  liststr          -> Get process list as string" << std::endl;
  std::cout << "  nodecheck <name> -> Check if ROS2 node is running" << std::endl;
  std::cout << "  startall         -> Start all test processes" << std::endl;
  std::cout << "  test             -> Run automated test sequence" << std::endl;
  std::cout << "  help             -> Show this help" << std::endl;
  std::cout << "  quit             -> Exit program" << std::endl;
  std::cout << "  Ctrl+C           -> Emergency stop all and exit" << std::endl;
  std::cout << std::endl;
  std::cout << "NOTE: Processes will open in separate terminal windows." << std::endl;
  std::cout << "      Use the stop command to properly terminate them." << std::endl;
  std::cout << std::endl;
}

int main()
{
  // Initialize ROS2
  rclcpp::init(0, nullptr);
  // Set up the signal handler
  signal(SIGINT, signalHandler);

  Logs::info("[main] Starting ROBOGait ProcessController Test");

  showHelp();

  std::string input;
  while (true)
  {
    std::cout << "process_controller_test> ";
    std::getline(std::cin, input);

    if (input == "quit" || input == "exit")
    {
      std::cout << "[QUIT] Quit command detected. Cleaning up processes..." << std::endl;
      g_processController.stopAllProcesses();
      std::cout << "[QUIT] Test finished." << std::endl;
      break;
    }

    if (input == "help")
    {
      showHelp();
      continue;
    }

    if (input == "list")
    {
      std::cout << "=== Current Process Status ===" << std::endl;
      g_processController.listProcesses();
      continue;
    }

    if (input == "liststr")
    {
      std::cout << "=== Process List String ===" << std::endl;
      std::string processStr = g_processController.listProcessesString();
      std::cout << "Returned string: '" << processStr << "'" << std::endl;
      continue;
    }

    if (input == "stopall")
    {
      std::cout << "Stopping all processes..." << std::endl;
      g_processController.stopAllProcesses();
      continue;
    }

    if (input == "startall")
    {
      std::cout << "Starting all test processes..." << std::endl;
      for (const auto& testProcess : g_testProcesses)
      {
        std::cout << "Starting '" << testProcess.name << "'..." << std::endl;
        g_processController.startProcess(testProcess.name, testProcess.executable, testProcess.arguments, testProcess.window_state);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
      continue;
    }

    if (input == "test")
    {
      std::cout << "=== Running Automated Test Sequence ===" << std::endl;

      // Test 1: Start and stop individual processes
      std::cout << "\n[TEST 1] Testing individual process start/stop..." << std::endl;
      g_processController.startProcess("echo_test", "echo", {"'Test 1 - Echo Process'"}, WindowState::MAXIMIZED);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      g_processController.listProcesses();
      g_processController.stopProcess("echo_test");
      std::cout << "[TEST 1] Individual process test completed." << std::endl;

      // Test 2: Test duplicate process prevention
      std::cout << "\n[TEST 2] Testing duplicate process prevention..." << std::endl;
      g_processController.startProcess("watch_test", "watch", {"-n", "3", "date"}, WindowState::MINIMIZED);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "Attempting to start same process again..." << std::endl;
      g_processController.startProcess("watch_test", "watch", {"-n", "1", "ls"}, WindowState::MAXIMIZED);
      g_processController.listProcesses();
      g_processController.stopProcess("watch_test");
      std::cout << "[TEST 2] Duplicate process prevention test completed." << std::endl;

      // Test 3: Test multiple processes with different window states
      std::cout << "\n[TEST 3] Testing multiple processes with different window states..." << std::endl;
      g_processController.startProcess("python_min", "python3", {"-c", "'import time; time.sleep(5); print(\"Minimized Python done\")'"},
                                       WindowState::MINIMIZED);
      g_processController.startProcess("python_max", "python3", {"-c", "'import time; time.sleep(5); print(\"Maximized Python done\")'"},
                                       WindowState::MAXIMIZED);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      g_processController.listProcesses();
      std::this_thread::sleep_for(std::chrono::seconds(4));
      g_processController.stopAllProcesses();
      std::cout << "[TEST 3] Multiple window states test completed." << std::endl;

      // Test 4: Test error conditions
      std::cout << "\n[TEST 4] Testing error conditions..." << std::endl;
      std::cout << "Testing empty executable..." << std::endl;
      g_processController.startProcess("empty_test", "", {"arg1"}, WindowState::MINIMIZED);
      std::cout << "Testing stop non-existent process..." << std::endl;
      g_processController.stopProcess("non_existent_process");
      std::cout << "[TEST 4] Error condition tests completed." << std::endl;

      // Test 5: Test ROS2 node checking (if available)
      std::cout << "\n[TEST 5] Testing ROS2 node functionality..." << std::endl;
      std::cout << "Checking if '/rosout' node is running..." << std::endl;
      bool nodeRunning = g_processController.isNodeRunning("/rosout");
      std::cout << "Node '/rosout' running: " << (nodeRunning ? "Yes" : "No") << std::endl;
      std::cout << "[TEST 5] ROS2 node functionality test completed." << std::endl;

      std::cout << "\n=== Automated Test Sequence Completed ===" << std::endl;
      continue;
    }

    // Parse command with arguments
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
    {
      tokens.push_back(token);
    }

    if (tokens.empty())
      continue;

    std::string command = tokens[0];

    if (command == "start" && tokens.size() == 2)
    {
      std::string identifier = tokens[1];

      // Try to parse as ID first
      try
      {
        int processId = std::stoi(identifier);
        if (processId >= 1 && processId <= static_cast<int>(g_testProcesses.size()))
        {
          const auto& testProcess = g_testProcesses[processId - 1];
          std::cout << "Starting process '" << testProcess.name << "'..." << std::endl;
          g_processController.startProcess(testProcess.name, testProcess.executable, testProcess.arguments, testProcess.window_state);
        }
        else
        {
          std::cout << "Process ID out of range. Use numbers 1-" << g_testProcesses.size() << std::endl;
        }
      }
      catch (const std::exception&)
      {
        // Not a number, treat as name
        bool found = false;
        for (const auto& testProcess : g_testProcesses)
        {
          if (testProcess.name == identifier)
          {
            std::cout << "Starting process '" << testProcess.name << "'..." << std::endl;
            g_processController.startProcess(testProcess.name, testProcess.executable, testProcess.arguments, testProcess.window_state);
            found = true;
            break;
          }
        }
        if (!found)
        {
          std::cout << "Process '" << identifier << "' not found in test processes." << std::endl;
        }
      }
    }
    else if (command == "stop" && tokens.size() == 2)
    {
      std::string processName = tokens[1];
      std::cout << "Stopping process '" << processName << "'..." << std::endl;
      g_processController.stopProcess(processName);
    }
    else if (command == "nodecheck" && tokens.size() == 2)
    {
      std::string nodeName = tokens[1];
      std::cout << "Checking if ROS2 node '" << nodeName << "' is running..." << std::endl;
      bool isRunning = g_processController.isNodeRunning(nodeName);
      std::cout << "Node '" << nodeName << "' is " << (isRunning ? "RUNNING" : "NOT RUNNING") << std::endl;
    }
    else
    {
      std::cout << "Unknown command or incorrect arguments. Type 'help' for usage." << std::endl;
    }
  }

  // Final cleanup
  g_processController.stopAllProcesses();

  Logs::info("[main] ROBOGait ProcessController Test finished");
  rclcpp::shutdown();

  std::cout << "Test finished." << std::endl;
  return 0;
}