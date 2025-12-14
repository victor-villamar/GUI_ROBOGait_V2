#include <boost/asio/signal_set.hpp>
#include <filesystem>
#include <rclcpp/rclcpp.hpp>
#include <thread>

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("node_manager");

  boost::asio::io_context io_context;

  // Al hacer control c que no de error y se cierre de forma segura
  // When doing control c it does not give an error and closes safely
  boost::asio::io_context io_context_signal_control;
  boost::asio::signal_set signals(io_context_signal_control, SIGINT, SIGTERM);

  // Ejecuta Boost.Asio en un hilo separado
  std::thread boost_thread([&io_context_signal_control]() { io_context_signal_control.run(); });

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
