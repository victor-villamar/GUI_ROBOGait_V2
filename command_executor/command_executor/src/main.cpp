#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "CommandExecutor.hpp"

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ROBOGait::command::CommandExecutor>());
  rclcpp::shutdown();
  return 0;
}
