#pragma once

#include <iostream>
#include <string>

#include <rclcpp/clock.hpp>
#include <rclcpp/logging.hpp>

namespace ROBOGait
{
namespace common
{
namespace logs
{

class Logs
{
  inline static const std::string ANSI_COLOR_RED = "\x1b[31m";
  inline static const std::string ANSI_COLOR_RESET = "\x1b[0m";

  inline static const auto logger = rclcpp::get_logger("global_logger");
  inline static bool use_ansi_colors = true;

public:
  /**
   * @brief Function to enable ANSI color output.
   */
  inline static void enableAnsiColors() { use_ansi_colors = true; }

  /**
   * @brief Function to disable ANSI color output.
   */
  inline static void disableAnsiColors() { use_ansi_colors = false; }

  template <typename... Ts> static void info(const char* format, Ts&&... args);
  template <typename... Ts> static void error(const char* format, Ts&&... args);

  template <class... T> static void infoStream(T&&... args);
  template <class... T> static void errorStream(T&&... args);

  template <typename... Ts> static void infoThrottle(const float period, const char* format, Ts&&... args);
  template <typename... Ts> static void errorThrottle(const float period, const char* format, Ts&&... args);
};

// Function implementations
template <typename... Ts> inline void Logs::info(const char* format, Ts&&... args) { RCLCPP_INFO(logger, format, args...); }

template <typename... Ts> inline void Logs::error(const char* format, Ts&&... args)
{
  if (use_ansi_colors)
  {
    std::string fmt{ANSI_COLOR_RED + std::string(format) + ANSI_COLOR_RESET};
    RCLCPP_ERROR(logger, fmt.c_str(), args...);
  }
  else
  {
    RCLCPP_ERROR(logger, format, args...);
  }
}

template <class... T> inline void Logs::infoStream(T&&... args)
{
  std::stringstream arguments;
  ((arguments << args), ...);
  RCLCPP_INFO_STREAM(logger, arguments.str());
}

template <class... T> inline void Logs::errorStream(T&&... args)
{
  std::stringstream arguments;
  ((arguments << args), ...);
  if (use_ansi_colors)
  {
    RCLCPP_ERROR_STREAM(logger, ANSI_COLOR_RED << arguments.str() << ANSI_COLOR_RESET);
  }
  else
  {
    RCLCPP_ERROR_STREAM(logger, arguments.str());
  }
}

template <typename... Ts> inline void Logs::infoThrottle(const float period, const char* format, Ts&&... args)
{
  static auto clock = std::make_shared<rclcpp::Clock>(RCL_STEADY_TIME);
  std::string fmt{std::string(format)};
  RCLCPP_INFO_THROTTLE(logger, *clock, static_cast<int64_t>(period * 1000), fmt.c_str(), std::forward<Ts>(args)...);
  std::cout << std::flush;
}

template <typename... Ts> inline void Logs::errorThrottle(const float period, const char* format, Ts&&... args)
{
  static auto clock = std::make_shared<rclcpp::Clock>(RCL_STEADY_TIME);
  if (use_ansi_colors)
  {
    std::string fmt{ANSI_COLOR_RED + std::string(format) + ANSI_COLOR_RESET};
    RCLCPP_ERROR_THROTTLE(logger, *clock, static_cast<int64_t>(period * 1000), fmt.c_str(), std::forward<Ts>(args)...);
    std::cout << std::flush;
  }
  else
  {
    std::string fmt{std::string(format)};
    RCLCPP_ERROR_THROTTLE(logger, *clock, static_cast<int64_t>(period * 1000), fmt.c_str(), std::forward<Ts>(args)...);
    std::cout << std::flush;
  }
}
} // namespace logs
} // namespace common
} // namespace VMB