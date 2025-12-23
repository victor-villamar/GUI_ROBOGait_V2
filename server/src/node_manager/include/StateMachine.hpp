#pragma once

#include <mutex>
#include <string>

#include "ProcessController.hpp"
#include "common/define.hpp"
#include "common/enum.hpp"
#include "utils/YamlLoader.hpp"

namespace ROBOGait
{
namespace state_machine
{
class StateMachine
{
public:
  /**
   * @brief Construct a new State Machine object
   */
  StateMachine();

  /**
   * @brief Destructor for the State Machine object
   */
  ~StateMachine();

  /**
   * @brief Initialize the State Machine
   *
   * @param param_dir Directory containing the configuration files
   * @return true if initialization was successful, false otherwise
   */
  bool init(std::string const& param_dir);

  /**
   * @brief Dispatch a process command
   *
   * @param command The process command to dispatch
   * @param start Whether to start or stop the process
   * @param map_name The name of the map (if applicable)
   * @return true if the command was successfully dispatched, false otherwise
   */
  bool dispatchProcessCommand(common::ProcessCommand command, bool start, const std::string& map_name);

private:
  /**
   * @brief Open the server database
   */
  void openServerDataBase();

  /**
   * @brief Close the server database
   */
  void closeServerDataBase();

  /**
   * @brief Start the robot
   */
  void startRobot();

  /**
   * @brief Stop the robot
   */
  void stopRobot();

  /**
   * @brief Start the bringup process with the specified map
   *
   * @param map_name Name of the map to use
   */
  void startBringup(std::string const& map_name);

  /**
   * @brief Reset the Node Manager state
   */
  void reset();

  /**
   * @brief Handle menu state requests
   *
   * @param start Whether to start or stop the process
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleMenuStateRequest(bool start);

  /**
   * @brief Handle remote controlled requests
   *
   * @param start Whether to start or stop the process
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleRemoteControlledRequest(bool start);

  /**
   * @brief Handle delete map requests
   *
   * @param map_name The name of the map to delete
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleDeleteMapRequest(const std::string& map_name);

  /**
   * @brief Handle save map requests
   *
   * @param map_name The name of the map to save
   * @param start Whether to start or stop the process
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleSaveMapRequest(const std::string& map_name, bool start);

  /**
   * @brief Handle init bringup requests
   *
   * @param map_name The name of the map to use
   * @param start Whether to start or stop the process
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleInitBringupRequest(const std::string& map_name, bool start);

  /**
   * @brief Handle stop processes requests
   *
   * @param start Whether to start or stop the process
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleStopProcessesRequest(bool start);

  /**
   * @brief Handle start database requests
   *
   * @param start Whether to start or stop the database
   * @return true if the request was successfully handled, false otherwise
   */
  bool handleStartDatabaseRequest(bool start);

  /**
   * @brief Stop bringup processes
   */
  void stopBringupProcesses();

  controller::ProcessController process_controller_; /**< Process controller instance */
  loader::YamlLoader yaml_loader_;                   /**< YAML loader instance */

  bool slam_launch_file;        /**< Flag indicating if the SLAM launch file is running */
  bool bringup_launch_file;     /**< Flag indicating if the bringup launch file is running */
  bool start_robot_launch_file; /**< Flag indicating if the start robot launch file is running */
  bool server_database_active;  /**< Flag indicating if the server database is active */
  bool navegando_;              /**< Flag indicating navigation mode */

  std::mutex state_mutex_; /**< Mutex for state synchronization */
};
} // namespace state_machine
} // namespace ROBOGait
