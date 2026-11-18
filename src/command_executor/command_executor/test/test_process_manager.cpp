#include <chrono>
#include <filesystem>
#include <future>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "ProcessManager.hpp"

namespace
{
using namespace std::chrono_literals;
using ROBOGait::command::ProcessManager;

constexpr auto GRACEFUL_COMMAND = "trap 'exit 0' INT; while true; do sleep 0.05; done";
constexpr auto SECOND_COMMAND = "trap 'exit 0' INT; while true; do sleep 0.05; done # second";

bool waitForFile(const std::filesystem::path& path, std::chrono::milliseconds timeout)
{
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (std::chrono::steady_clock::now() < deadline)
  {
    if (std::filesystem::exists(path))
    {
      return true;
    }
    std::this_thread::sleep_for(10ms);
  }
  return std::filesystem::exists(path);
}
} // namespace

TEST(ProcessManagerTest, StopUnknownProcessFails)
{
  ProcessManager manager;

  EXPECT_FALSE(manager.stopProcess("unknown command"));
}

TEST(ProcessManagerTest, OneShotCommandReturnsItsExecutionResult)
{
  ProcessManager manager;

  EXPECT_TRUE(manager.executeOneShotCommand("true"));
  EXPECT_FALSE(manager.executeOneShotCommand("false"));
}

TEST(ProcessManagerTest, RunningProcessCannotBeStartedTwiceAndCanBeRestartedAfterStop)
{
  ProcessManager manager;

  ASSERT_TRUE(manager.startProcess(GRACEFUL_COMMAND));
  EXPECT_FALSE(manager.startProcess(GRACEFUL_COMMAND));
  ASSERT_TRUE(manager.stopProcess(GRACEFUL_COMMAND));

  ASSERT_TRUE(manager.startProcess(GRACEFUL_COMMAND));
  EXPECT_TRUE(manager.stopProcess(GRACEFUL_COMMAND));
}

TEST(ProcessManagerTest, StopInProgressDoesNotBlockOperationsOnOtherProcesses)
{
  ProcessManager manager;
  const auto ready_file = std::filesystem::temp_directory_path() / "process_manager_ignore_sigint_ready";
  std::filesystem::remove(ready_file);

  const std::string ignore_sigint_command = "trap '' INT; touch '" + ready_file.string() + "'; while true; do sleep 0.05; done";
  ASSERT_TRUE(manager.startProcess(ignore_sigint_command));
  ASSERT_TRUE(waitForFile(ready_file, 1s));

  auto stop_future = std::async(std::launch::async, [&manager, &ignore_sigint_command]() { return manager.stopProcess(ignore_sigint_command); });

  std::this_thread::sleep_for(100ms);
  EXPECT_FALSE(manager.stopProcess(ignore_sigint_command));

  const auto start_time = std::chrono::steady_clock::now();
  ASSERT_TRUE(manager.startProcess(SECOND_COMMAND));
  const auto start_elapsed = std::chrono::steady_clock::now() - start_time;

  EXPECT_LT(start_elapsed, 500ms);
  EXPECT_TRUE(manager.stopProcess(SECOND_COMMAND));

  ASSERT_EQ(stop_future.wait_for(2s), std::future_status::ready);
  EXPECT_TRUE(stop_future.get());

  std::filesystem::remove(ready_file);
}
