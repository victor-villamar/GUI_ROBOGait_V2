#include "Logs.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

using namespace ROBOGait::common::logs;

int main()
{
  rclcpp::init(0, nullptr);

  // Basic tests of all logging functions
  std::string message = "This is a test message";
  Logs::infoStream("[main]" + message + " in infoStream format");
  Logs::errorStream("[main]" + message + " in errorStream format");

  Logs::info("[main] This is a info test, %d", 42);
  Logs::error("[main] This is an error test, %s", message.c_str());

  std::cout << "====== THROTTLE TEST @ 20HZ for 10 seconds ======" << std::endl;
  std::cout << "Throttle period: 2 seconds" << std::endl;
  std::cout << "Expect: ~5 throttled messages (every 2 seconds)" << std::endl;

  const double frecuency_hz = 20.0;
  const double test_duration_sec = 10.0;
  const double throttle_period = 2.0f;
  const int total_iterations = static_cast<int>(frecuency_hz * test_duration_sec);
  const int sleep_ms = static_cast<int>(1000.0 / frecuency_hz);

  auto start_time = std::chrono::steady_clock::now();

  for (int i = 0; i < total_iterations; ++i)
  {
    Logs::infoThrottle(throttle_period, "[main] This is a throttled info test, iteration %d", i);
    Logs::errorThrottle(throttle_period, "[main] This is a throttled error test, iteration %d", i);

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
  }

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::cout << "======THROTTLE TEST RESULTS ======" << std::endl;
  std::cout << "Test completed in " << duration.count() << " ms" << std::endl;
  std::cout << "Calculation: " << (test_duration_sec) << " seconds / " << (throttle_period) << " throttle = ~" << (test_duration_sec / throttle_period)
            << " messages" << std::endl;

  rclcpp::shutdown();
  return 0;
}