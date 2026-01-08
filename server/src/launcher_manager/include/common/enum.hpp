#pragma once

namespace ROBOGait
{
namespace common
{
/**
 * @brief Enum class representing the various states of a process.
 */
enum class ProcessState
{
  STOPPED, /**< Process is stopped */
  RUNNING, /**< Process is running */
  UNKNOWN  /**< Process state is unknown */
};

/**
 * @brief Enum class representing the window states.
 */
enum class WindowState
{
  NOT_DEFINED = -1, /**< Not defined */
  MINIMIZED = 0,    /**< Window is minimized */
  MAXIMIZED = 1     /**< Window is maximized */
};

/**
 * @brief Enum representing GUI requests for launching/stopping processes.
 */
enum class ProcessCommand
{
  START_MENU = 1,
  REMOTE_CONTROLLED = 2,
  DELETE_MAP = 3,
  SAVE_MAP = 4,
  INIT_BRINGUP = 5,
  STOP_PROCESS = 6,
  OPEN_DATABASE = 7,
};
} // namespace common
} // namespace ROBOGait
