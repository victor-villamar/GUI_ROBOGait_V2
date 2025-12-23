#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>
#include <logs/Logs.hpp>
#include <vector>

#include "StateMachine.hpp"
#include "common/functions.hpp"

using namespace ROBOGait::common;
using namespace ROBOGait::common::logs;
using namespace ROBOGait::state_machine;

StateMachine::StateMachine() :
    server_database_active(false), slam_launch_file(false), bringup_launch_file(false), start_robot_launch_file(false), navegando_(NAV)
{
  Logs::infoStream("[StateMachine::StateMachine] Creating StateMachine");
}

StateMachine::~StateMachine()
{
  Logs::infoStream("[StateMachine::~StateMachine] Destroying StateMachine");
  closeServerDataBase();
  reset();
}

bool StateMachine::init(std::string const& param_dir)
{
  if (!yaml_loader_.loadFromFile(param_dir))
  {
    Logs::error("[StateMachine::init] Could not load configuration file.");
    return false;
  }
  Logs::infoStream("[StateMachine::init] Configuration file loaded successfully.");
  return true;
}

void StateMachine::openServerDataBase()
{
  auto database_config = yaml_loader_.getDatabaseConfig();
  if (process_controller_.listProcessesString().find(database_config.name) != std::string::npos)
  {
    Logs::infoStream("[StateMachine::openServerDataBase] Database is already running.");
    return;
  }

  std::vector<std::string> args = {database_config.script, database_config.path};
  process_controller_.startProcess(database_config.name, "python3", args, WindowState::MINIMIZED);
  server_database_active = true;
}

void StateMachine::closeServerDataBase()
{
  if (server_database_active)
  {
    Logs::infoStream("[StateMachine::closeServerDataBase] Closing database.");
    auto database_config = yaml_loader_.getDatabaseConfig();
    process_controller_.stopProcess(database_config.name);
    server_database_active = false;
  }
  else
  {
    Logs::infoStream("[StateMachine::closeServerDataBase] Database is not active.");
  }
}

void StateMachine::startRobot()
{
#if !EN_CASA

  if (!start_robot_launch_file)
  {
    auto start_robot_definition = yaml_loader_.getProcess("START_ROBOT");
    auto tf_service_definition = yaml_loader_.getProcess("TF_SERVICE");
    if (!start_robot_definition || !tf_service_definition)
    {
      return;
    }

    process_controller_.startProcess(start_robot_definition->name, start_robot_definition->executable, start_robot_definition->arguments,
                                     start_robot_definition->window_state);
    process_controller_.startProcess(tf_service_definition->name, tf_service_definition->executable, tf_service_definition->arguments,
                                     tf_service_definition->window_state);
    start_robot_launch_file = true;
  }
#endif
}

void StateMachine::stopRobot()
{
  //   if (!tf_service_client_)
  //   {
  //     tf_service_client_.reset();
  //     RCLCPP_INFO(node_manager->get_logger(), "Close Robot Pose Client destroy.");
  //   }

  if (start_robot_launch_file)
  {
    auto start_robot_definition = yaml_loader_.getProcess("START_ROBOT");
    if (start_robot_definition)
    {
      process_controller_.stopProcess(start_robot_definition->name);
    }
    start_robot_launch_file = false;
  }
}

void StateMachine::startBringup(std::string const& map_name)
{
#if !EN_CASA

  if (!bringup_launch_file)
  {
    const std::string path_to_maps = yaml_loader_.getPathToMaps();
    const std::string map_yaml_path = path_to_maps + "/" + map_name + ".yaml";
    bool started_any = false;
    if (navegando_)
    {
      auto nav2_definition = yaml_loader_.getProcess("NAV2_BRINGUP");
      if (nav2_definition)
      {
        auto args = nav2_definition->arguments;
        if (!args.empty() && args.back() == "map:=")
        {
          args.back() += map_yaml_path;
        }
        else
        {
          args.push_back("map:=" + map_yaml_path);
        }
        process_controller_.startProcess(nav2_definition->name, nav2_definition->executable, args, nav2_definition->window_state);
        started_any = true;
      }
    }
    else
    {
      if (auto start_robot_definition = yaml_loader_.getProcess("START_ROBOT"))
      {
        process_controller_.stopProcess(start_robot_definition->name);
        start_robot_launch_file = false;
      }

      if (auto nav_robot_definition = yaml_loader_.getProcess("NAVIGATION_ROBOT"))
      {
        auto args = nav_robot_definition->arguments;
        if (!args.empty() && args.back() == "map:=")
        {
          args.back() += map_yaml_path;
        }
        else
        {
          args.push_back("map:=" + map_yaml_path);
        }
        process_controller_.startProcess(nav_robot_definition->name, nav_robot_definition->executable, args, nav_robot_definition->window_state);
        started_any = true;
      }

      const std::vector<std::string> auxiliary_processes = {"DISTANCE_TRACKER", "DISTANCE_CONTROLLER", "CAMERA_CONTROLLER", "DYNAMIXEL", "PRUEBA"};
      for (const auto& identifier : auxiliary_processes)
      {
        if (auto definition = yaml_loader_.getProcess(identifier))
        {
          process_controller_.startProcess(definition->name, definition->executable, definition->arguments, definition->window_state);
          started_any = true;
        }
      }
    }
    bringup_launch_file = started_any;
  }
#endif
}

void StateMachine::reset()
{

#if !EN_CASA
  slam_launch_file = false;
  bringup_launch_file = false;
  start_robot_launch_file = false;
  process_controller_.listProcesses();
  process_controller_.stopAllProcesses();
#endif
}

bool StateMachine::dispatchProcessCommand(common::ProcessCommand command, bool start, const std::string& map_name)
{
  std::scoped_lock lock(state_mutex_);

  switch (command)
  {
    case common::ProcessCommand::START_MENU:
      return handleMenuStateRequest(start);
    case common::ProcessCommand::REMOTE_CONTROLLED:
      return handleRemoteControlledRequest(start);
    case common::ProcessCommand::DELETE_MAP:
      return handleDeleteMapRequest(map_name);
    case common::ProcessCommand::SAVE_MAP:
      return handleSaveMapRequest(map_name, start);
    case common::ProcessCommand::INIT_BRINGUP:
      return handleInitBringupRequest(map_name, start);
    case common::ProcessCommand::STOP_PROCESS:
      return handleStopProcessesRequest(start);
    case common::ProcessCommand::OPEN_DATABASE:
      return handleStartDatabaseRequest(start);
    default:
      Logs::error("[StateMachine::dispatchProcessCommand] Unknown process id: %u", static_cast<unsigned>(command));
      return false;
  }
}

bool StateMachine::handleMenuStateRequest(bool start)
{
  if (start)
  {
    if (bringup_launch_file)
    {
      stopBringupProcesses();
    }
    startRobot();
    return true;
  }
  else
  {
    // stopRobot(); TODO: Review if we want to stop the robot when stopping from menu
    return true;
  }
}

bool StateMachine::handleRemoteControlledRequest(bool start)
{
#if !EN_CASA
  auto cartographer_definition = yaml_loader_.getProcess("CARTOGRAPHER");
  if (!cartographer_definition)
  {
    Logs::error("Process configuration '%s' not found", "CARTOGRAPHER");
    return false;
  }

  // Start the cartographer process
  if (start)
  {
    if (!slam_launch_file)
    {
      process_controller_.startProcess(cartographer_definition->name, cartographer_definition->executable, cartographer_definition->arguments,
                                       cartographer_definition->window_state);
      slam_launch_file = true;
    }
    return true;
  }
  // Stop the cartographer process and delete temporary map files
  else
  {
    if (slam_launch_file)
    {
      process_controller_.stopProcess(cartographer_definition->name);
      slam_launch_file = false;
    }

    const std::string maps_path = yaml_loader_.getPathToMaps();
    if (!maps_path.empty())
    {
      common::functions::deleteMapFile(maps_path, "temporal_map");
    }

    return true;
  }
#endif

  (void)start;
  return true;
}

bool StateMachine::handleDeleteMapRequest(const std::string& map_name)
{
  if (map_name.empty())
  {
    Logs::error("[StateMachine::handleDeleteMapRequest] Map name is empty.");
    return false;
  }

  const std::string maps_path = yaml_loader_.getPathToMaps();
  if (maps_path.empty())
  {
    Logs::error("[StateMachine::handleDeleteMapRequest] Maps path is empty.");
    return false;
  }

  common::functions::deleteMapFile(maps_path, map_name);
  return true;
}

bool StateMachine::handleSaveMapRequest(const std::string& map_name, bool start)
{
  const std::string maps_dir = yaml_loader_.getPathToMaps();
  if (maps_dir.empty())
  {
    Logs::error("[StateMachine::handleSaveMapRequest] Maps path is empty.");
    return false;
  }

  auto maps_saver_definition = yaml_loader_.getProcess("MAP_SAVER");
  if (!maps_saver_definition)
  {
    Logs::error("Process configuration '%s' not found", "MAP_SAVER");
    return false;
  }

  if (!start)
  {
    process_controller_.stopProcess(maps_saver_definition->name);
    return true;
  }
  else
  {

    if (map_name.empty())
    {
      Logs::error("[StateMachine::handleSaveMapRequest] Map name is empty.");
      return false;
    }

    const std::string output_prefix = maps_dir + "/" + common::functions::replaceSpaces(map_name);

    if (process_controller_.listProcessesString().find(maps_saver_definition->name) != std::string::npos)
    {
      process_controller_.stopProcess(maps_saver_definition->name);
    }

    maps_saver_definition->arguments.push_back(output_prefix);

    process_controller_.startProcess(maps_saver_definition->name, maps_saver_definition->executable, maps_saver_definition->arguments,
                                     maps_saver_definition->window_state);
    return true;
  }
}

bool StateMachine::handleInitBringupRequest(const std::string& map_name, bool start)
{
  if (start)
  {
    const std::string map_name_without_spaces = common::functions::replaceSpaces(map_name);
    if (map_name_without_spaces.empty())
    {
      Logs::error("[StateMachine::handleInitBringupRequest] Map name is empty.");
      return false;
    }
    startBringup(map_name_without_spaces);
  }
  else
  {
    stopBringupProcesses();
  }
  return true;
}

bool StateMachine::handleStopProcessesRequest(bool start)
{
  if (start)
  {
    reset();
    return true;
  }
  else
  {
    return false;
  }
}

bool StateMachine::handleStartDatabaseRequest(bool start)
{
  if (start)
  {
    openServerDataBase();
    return true;
  }
  else
  {
    closeServerDataBase();
    return false;
  }
}

void StateMachine::stopBringupProcesses()
{
#if !EN_CASA
  if (navegando_)
  {
    if (auto nav2_definition = yaml_loader_.getProcess("NAV2_BRINGUP"))
    {
      process_controller_.stopProcess(nav2_definition->name);
    }
  }
  else
  {
    const std::vector<std::string> processes = {"NAVIGATION_ROBOT", "DISTANCE_TRACKER", "DISTANCE_CONTROLLER", "CAMERA_CONTROLLER", "DYNAMIXEL", "PRUEBA"};
    for (const auto& identifier : processes)
    {
      if (auto definition = yaml_loader_.getProcess(identifier))
      {
        process_controller_.stopProcess(definition->name);
      }
    }
  }
#endif

  bringup_launch_file = false;
}
