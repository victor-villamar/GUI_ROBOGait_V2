#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <rclcpp/executors.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/qos.hpp>
#include <rclcpp/utilities.hpp>

#include <command_executor_msgs/msg/robot_status.hpp>
#include <command_executor_msgs/srv/cmd.hpp>

#include "CommandExecutor.hpp"
#include "Define.hpp"

namespace
{
using ROBOGait::command::CommandExecutor;
using namespace std::chrono_literals;

std::filesystem::path createConfigFile(const std::string& test_name)
{
  const std::filesystem::path test_dir = std::filesystem::temp_directory_path() / ("command_executor_ros_test_" + test_name);
  std::filesystem::create_directories(test_dir);

  const std::filesystem::path config_path = test_dir / "config.yaml";
  std::ofstream config_file(config_path);
  config_file << "command_executor:\n";
  config_file << "  ros__parameters:\n";
  config_file << "    robot_info:\n";
  config_file << "      id: 1\n";
  config_file << "      namespace: \"ROBOGait\"\n";
  config_file << "      version: \"V3\"\n";
  config_file << "      hardware_id: 12345678\n";
  config_file << "      serial_number: \"ROBOGait-001\"\n";
  config_file << "\n";
  config_file << "    allow_list:\n";
  config_file << "      - action\n";
  config_file << "      - topic\n";
  config_file << "      - delete\n";
  config_file.close();

  return config_path;
}

void spinUntil(rclcpp::Executor& executor, const std::function<bool()>& predicate, const std::chrono::milliseconds timeout)
{
  const auto start_time = std::chrono::steady_clock::now();
  while (!predicate() && std::chrono::steady_clock::now() - start_time < timeout)
  {
    executor.spin_some(50ms);
  }
}
} // namespace

class CommandExecutorRosTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    if (!rclcpp::ok())
    {
      rclcpp::init(0, nullptr);
    }
  }

  static void TearDownTestSuite()
  {
    if (rclcpp::ok())
    {
      rclcpp::shutdown();
    }
  }
};

TEST_F(CommandExecutorRosTest, InitializeFailsWithoutRequiredParameters)
{
  auto command_executor = std::make_shared<CommandExecutor>(rclcpp::NodeOptions());

  EXPECT_FALSE(command_executor->initialize());
}

TEST_F(CommandExecutorRosTest, InitializeWithConfigAndRejectInvalidCommand)
{
  const std::filesystem::path config_path = createConfigFile("valid_config");
  rclcpp::NodeOptions options;
  options.arguments({"--ros-args", "--params-file", config_path.string()});

  auto command_executor = std::make_shared<CommandExecutor>(options);
  ASSERT_TRUE(command_executor->initialize());

  auto test_node = std::make_shared<rclcpp::Node>("command_executor_ros_test");
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(command_executor);
  executor.add_node(test_node);

  auto client = test_node->create_client<command_executor_msgs::srv::Cmd>(ROBOGait::command::define::S_CMD);
  ASSERT_TRUE(client->wait_for_service(2s));

  auto request = std::make_shared<command_executor_msgs::srv::Cmd::Request>();
  request->cmd = "echo rejected";
  request->execute = true;

  auto response_future = client->async_send_request(request);
  const auto future_status = executor.spin_until_future_complete(response_future, 2s);
  ASSERT_EQ(future_status, rclcpp::FutureReturnCode::SUCCESS);
  EXPECT_FALSE(response_future.get()->success);

  std::promise<command_executor_msgs::msg::RobotStatus> robot_status_promise;
  auto robot_status_future = robot_status_promise.get_future();
  bool robot_status_received = false;

  auto subscription = test_node->create_subscription<command_executor_msgs::msg::RobotStatus>(
      ROBOGait::command::define::T_ROBOT_STATUS, rclcpp::QoS(1).best_effort(),
      [&](const command_executor_msgs::msg::RobotStatus::SharedPtr msg)
      {
        if (!robot_status_received)
        {
          robot_status_received = true;
          robot_status_promise.set_value(*msg);
        }
      });

  spinUntil(executor, [&]() { return robot_status_received; }, 2s);
  ASSERT_TRUE(robot_status_received);

  const command_executor_msgs::msg::RobotStatus robot_status = robot_status_future.get();
  EXPECT_EQ(robot_status.id, 1);
  EXPECT_EQ(robot_status.ns, "ROBOGait");
  EXPECT_EQ(robot_status.version, "V3");
  EXPECT_EQ(robot_status.hardware_id, 12345678u);
  EXPECT_EQ(robot_status.serial_number, "ROBOGait-001");

  executor.remove_node(test_node);
  executor.remove_node(command_executor);
  std::filesystem::remove_all(config_path.parent_path());
}
