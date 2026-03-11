#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "CommandExecutor.hpp"

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto command_executor = std::make_shared<ROBOGait::command::CommandExecutor>();

  if (!command_executor->initialize())
  {
    RCLCPP_ERROR(rclcpp::get_logger("command_executor"), "[main] Failed to initialize CommandExecutor");
    return 1;
  }

  rclcpp::spin(command_executor);
  rclcpp::shutdown();
  return 0;
}
