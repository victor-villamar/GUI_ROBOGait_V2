#include <memory>

#include <rclcpp/executors/multi_threaded_executor.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/utilities.hpp>

#include "CommandExecutor.hpp"

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto command_executor = std::make_shared<ROBOGait::command::CommandExecutor>(rclcpp::NodeOptions());

  if (!command_executor->initialize())
  {
    RCLCPP_ERROR(rclcpp::get_logger("command_executor"), "[main] Failed to initialize CommandExecutor");
    return 1;
  }

  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 2);
  executor.add_node(command_executor);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
