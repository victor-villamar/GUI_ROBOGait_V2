#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

#include "../include/common/functions.hpp"
#include "../include/utils/YamlLoader.hpp"

using namespace ROBOGait::loader;
using namespace ROBOGait::common;

// Global variables for cleanup
std::unique_ptr<YamlLoader> g_yaml_loader;

// Function to clean up
void cleanup()
{
  std::cout << "\n[CLEANUP] Cleaning up YamlLoader test..." << std::endl;
  g_yaml_loader.reset();
  std::cout << "[CLEANUP] Cleanup completed." << std::endl;
}

// Signal handler for Ctrl+C
void signalHandler(int signal)
{
  if (signal == SIGINT)
  {
    std::cout << "\n[SIGNAL] Received SIGINT (Ctrl+C). Cleaning up..." << std::endl;
    cleanup();
    exit(0);
  }
}

void showMenu()
{
  std::cout << "\n=== YamlLoader Test Menu ===" << std::endl;
  std::cout << "1. Load configuration file" << std::endl;
  std::cout << "2. Test getProcess() for all processes" << std::endl;
  std::cout << "3. Test getProcess() for specific process" << std::endl;
  std::cout << "4. Test getDatabaseConfig()" << std::endl;
  std::cout << "5. Test getPathToMaps()" << std::endl;
  std::cout << "6. Test getType()" << std::endl;
  std::cout << "7. Test invalid process (should fail)" << std::endl;
  std::cout << "8. Run automated test sequence" << std::endl;
  std::cout << "9. Show this menu" << std::endl;
  std::cout << "0. Exit" << std::endl;
  std::cout << "Choice: ";
}

void testLoadConfiguration()
{
  std::cout << "\n--- Testing loadFromFile() ---" << std::endl;

  const std::string config_path = functions::getFilePath("node_manager", "param/");

  if (config_path.empty())
  {
    std::cout << "Failed to get configuration path" << std::endl;
    return;
  }

  std::cout << "Using config path: " << config_path << std::endl;

  bool result = g_yaml_loader->loadFromFile(config_path);

  if (result)
  {
    std::cout << "Configuration loaded successfully" << std::endl;
  }
  else
  {
    std::cout << "Failed to load configuration" << std::endl;
  }
}

void testGetAllProcesses()
{
  std::cout << "\n--- Testing getProcess() for all processes ---" << std::endl;

  const std::vector<std::string> process_keys = {"START_ROBOT",      "TF_SERVICE",        "CARTOGRAPHER", "NAV2_BRINGUP", "DISTANCE_CONTROLLER",
                                                 "DISTANCE_TRACKER", "CAMERA_CONTROLLER", "DYNAMIXEL",    "PRUEBA"};

  for (const auto& key : process_keys)
  {
    auto process_config = g_yaml_loader->getProcess(key);

    if (process_config)
    {
      std::cout << "Process '" << key << "':" << std::endl;
      std::cout << "   Name: " << process_config->name << std::endl;
      std::cout << "   Executable: " << process_config->executable << std::endl;
      std::cout << "   Arguments: ";
      for (const auto& arg : process_config->arguments)
      {
        std::cout << arg << " ";
      }
      std::cout << std::endl;
      std::cout << "   Window State: " << static_cast<int>(process_config->window_state) << std::endl;
    }
    else
    {
      std::cout << "Failed to get process configuration for '" << key << "'" << std::endl;
    }
  }
}

void testGetSpecificProcess()
{
  std::cout << "\n--- Testing getProcess() for specific process ---" << std::endl;
  std::cout << "Enter process key (e.g., START_ROBOT, TF_SERVICE): ";

  std::string key;
  std::getline(std::cin, key);

  if (key.empty())
  {
    std::cout << "Empty key provided" << std::endl;
    return;
  }

  auto process_config = g_yaml_loader->getProcess(key);

  if (process_config)
  {
    std::cout << "Process '" << key << "' found:" << std::endl;
    std::cout << "   Name: " << process_config->name << std::endl;
    std::cout << "   Executable: " << process_config->executable << std::endl;
    std::cout << "   Arguments: ";
    for (const auto& arg : process_config->arguments)
    {
      std::cout << arg << " ";
    }
    std::cout << std::endl;

    std::string window_state_str;
    switch (process_config->window_state)
    {
      case WindowState::MINIMIZED:
        window_state_str = "MINIMIZED";
        break;
      case WindowState::MAXIMIZED:
        window_state_str = "MAXIMIZED";
        break;
      default:
        window_state_str = "UNKNOWN";
        break;
    }
    std::cout << "   Window State: " << window_state_str << std::endl;
  }
  else
  {
    std::cout << "Process '" << key << "' not found" << std::endl;
  }
}

void testGetDatabaseConfig()
{
  std::cout << "\n--- Testing getDatabaseConfig() ---" << std::endl;

  try
  {
    auto db_config = g_yaml_loader->getDatabaseConfig();

    std::cout << "Database configuration:" << std::endl;
    std::cout << "   Name: " << db_config.name << std::endl;
    std::cout << "   Path: " << db_config.path << std::endl;
    std::cout << "   Script: " << db_config.script << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cout << "Failed to get database configuration: " << e.what() << std::endl;
  }
}

void testGetPathToMaps()
{
  std::cout << "\n--- Testing getPathToMaps() ---" << std::endl;

  try
  {
    std::string maps_path = g_yaml_loader->getPathToMaps();

    if (!maps_path.empty())
    {
      std::cout << "Maps path: " << maps_path << std::endl;
    }
    else
    {
      std::cout << "Empty maps path returned" << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cout << "Failed to get maps path: " << e.what() << std::endl;
  }
}

void testGetType()
{
  std::cout << "\n--- Testing getType() ---" << std::endl;

  try
  {
    std::string type = g_yaml_loader->getType();

    if (!type.empty())
    {
      std::cout << "Robot type: " << type << std::endl;
    }
    else
    {
      std::cout << "Empty type returned (this might be expected)" << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cout << "Failed to get type: " << e.what() << std::endl;
  }
}

void testInvalidProcess()
{
  std::cout << "\n--- Testing getProcess() with invalid key ---" << std::endl;

  const std::string invalid_key = "INVALID_PROCESS_KEY";
  auto process_config = g_yaml_loader->getProcess(invalid_key, false);

  if (!process_config)
  {
    std::cout << "Correctly returned nullopt for invalid key '" << invalid_key << "'" << std::endl;
  }
  else
  {
    std::cout << "Unexpectedly found configuration for invalid key '" << invalid_key << "'" << std::endl;
  }
}

void runAutomatedTests()
{
  std::cout << "\n=== Running Automated Test Sequence ===" << std::endl;

  // Test 1: Load configuration
  std::cout << "\n[1/6] Testing configuration loading..." << std::endl;
  testLoadConfiguration();

  // Test 2: Get all processes
  std::cout << "\n[2/6] Testing all processes..." << std::endl;
  testGetAllProcesses();

  // Test 3: Database configuration
  std::cout << "\n[3/6] Testing database configuration..." << std::endl;
  testGetDatabaseConfig();

  // Test 4: Maps path
  std::cout << "\n[4/6] Testing maps path..." << std::endl;
  testGetPathToMaps();

  // Test 5: Robot type
  std::cout << "\n[5/6] Testing robot type..." << std::endl;
  testGetType();

  // Test 6: Invalid process
  std::cout << "\n[6/6] Testing invalid process..." << std::endl;
  testInvalidProcess();

  std::cout << "\nAutomated test sequence completed!" << std::endl;
}

int main()
{
  // Set up the signal handler
  signal(SIGINT, signalHandler);

  Logs::info("[main] Starting ROBOGait YamlLoader Test");

  // Create YamlLoader instance
  g_yaml_loader = std::make_unique<YamlLoader>();

  std::cout << "=== ROBOGait YamlLoader Test ===" << std::endl;
  std::cout << "This test validates YamlLoader functionality with config.yaml" << std::endl;

  // Show initial menu
  showMenu();

  std::string input;
  while (true)
  {
    std::getline(std::cin, input);

    if (input.empty())
    {
      continue;
    }

    try
    {
      int choice = std::stoi(input);

      switch (choice)
      {
        case 1:
        {
          testLoadConfiguration();
          break;
        }
        case 2:
        {
          testGetAllProcesses();
          break;
        }
        case 3:
        {
          testGetSpecificProcess();
          break;
        }
        case 4:
        {
          testGetDatabaseConfig();
          break;
        }
        case 5:
        {
          testGetPathToMaps();
          break;
        }
        case 6:
        {
          testGetType();
          break;
        }
        case 7:
        {
          testInvalidProcess();
          break;
        }
        case 8:
        {
          runAutomatedTests();
          break;
        }
        case 9:
        {
          showMenu();
          break;
        }
        case 0:
        {
          std::cout << "Exiting YamlLoader test..." << std::endl;
          cleanup();
          return 0;
        }
        default:
        {
          std::cout << "Invalid option. Please choose 0-9." << std::endl;
          break;
        }
      }
    }
    catch (const std::exception& e)
    {
      std::cout << "Invalid input: " << e.what() << std::endl;
    }

    std::cout << "\nPress Enter for menu or enter choice directly: ";
  }

  return 0;
}