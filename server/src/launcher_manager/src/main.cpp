#include <filesystem>
#include <memory>
#include <rclcpp/executors.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp/node.hpp>
#include <thread>

#include "LauncherManager.hpp"

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  auto parent_node = rclcpp::Node::make_shared("launcher_manager");

  ROBOGait::manager::LauncherManager launcher_manager(parent_node.get());
  if (!launcher_manager.init())
  {
    RCLCPP_ERROR(parent_node->get_logger(), "Failed to initialize LauncherManager");
    return 1;
  }

  rclcpp::spin(parent_node);
  rclcpp::shutdown();
  return 0;
}
