#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <string>

#include "ProcessController.h"

#include <rclcpp/rclcpp.hpp>

#include <yaml-cpp/yaml.h> // leer el archivo yaml

class NodeManager
{
public:
    NodeManager(rclcpp::Node::SharedPtr node_ptr);
    void open_server_database();
    void close_server_database();


    void start_robot();
    void stop_robot();

    void start_bringup(std::string const &map_name);

    void reset();


private:

    bool server_database_active = false;

    rclcpp::Node::SharedPtr node_manager;
    ProcessController processController;

    bool slam_launch_file = false;
    bool bringup_launch_file = false;
    bool start_robot_launch_file = false;

};

#endif // NODEMANAGER_H