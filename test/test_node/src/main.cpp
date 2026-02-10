#include <cstdlib>
#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <string>

#include "TestNode.hpp"

/**
 * @brief Main function for test_node
 *
 * Prompts the user to enter a ROS_DOMAIN_ID, then initializes
 * the ROS2 node with that domain and starts publishing robot status.
 */
int main(int argc, char** argv)
{
  int domain_id;
  std::cout << "===============================================" << std::endl;
  std::cout << "     TEST NODE - Robot Status Publisher       " << std::endl;
  std::cout << "===============================================" << std::endl;
  std::cout << std::endl;
  std::cout << "Ingrese el ROS_DOMAIN_ID (0-101): ";
  std::cin >> domain_id;

  // Validate domain_id
  if (domain_id < 0 || domain_id > 101)
  {
    std::cerr << "Error: ROS_DOMAIN_ID debe estar entre 0 y 101" << std::endl;
    return 1;
  }

  // Set ROS_DOMAIN_ID environment variable
  std::string domain_str = std::to_string(domain_id);
  setenv("ROS_DOMAIN_ID", domain_str.c_str(), 1);

  std::cout << std::endl;
  std::cout << "ROS_DOMAIN_ID configurado a: " << domain_id << std::endl;
  std::cout << std::endl;

  std::string robot_namespace = "";
  std::cout << "Quiere lanzar el nodo con namespace? (s/n): ";
  char launch_with_namespace;
  std::cin >> launch_with_namespace;

  if (launch_with_namespace == 's' || launch_with_namespace == 'S')
  {
    std::cout << "Ingrese el robot namespace: ";
    std::cin >> robot_namespace;
  }

  // Initialize ROS2
  rclcpp::init(argc, argv);

  // Create node
  auto node = std::make_shared<rclcpp::Node>("test_node");

  RCLCPP_INFO(node->get_logger(), "[main] Node created with ROS_DOMAIN_ID: %d", domain_id);

  // Create TestNode instance (robot info is hardcoded inside)
  auto test_node = std::make_unique<TestNode>(node, robot_namespace);

  std::cout << "Nodo iniciado. Publicando en " << T_ROBOT_STATUS << " a 1Hz..." << std::endl;
  std::cout << "Presione Ctrl+C para detener." << std::endl;
  std::cout << std::endl;

  // Spin the node
  rclcpp::spin(node);

  // Cleanup
  rclcpp::shutdown();

  std::cout << std::endl;
  std::cout << "Nodo detenido." << std::endl;

  return 0;
}
