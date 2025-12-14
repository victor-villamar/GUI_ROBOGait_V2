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

} // namespace common
} // namespace ROBOGait