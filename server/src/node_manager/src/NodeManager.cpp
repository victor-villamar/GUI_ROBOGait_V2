#include "../include/NodeManager.h"

#include <iostream>

NodeManager::NodeManager(rclcpp::Node::SharedPtr node_ptr) : processController()
{
    node_manager = node_ptr;
}

void NodeManager::open_server_database()
{
    YAML::Node config;
    try
    {
        std::string path_ = PATH;
        config = YAML::LoadFile(path_ + "server/src/node_manager/param/config.yaml");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading YAML file: " << e.what() << std::endl;
    }

    if (processController.listProcessesString().find(config["NAME_DATABASE"].as<std::string>()) == std::string::npos)
    {
        std::string name_server_database = config["NAME_DATABASE"].as<std::string>();
        std::string path_server_database = PATH + config["DATABASE"].as<std::string>();
        pri1("Path to database: " + path_server_database);
        std::string path2comand = PATH + config["PATH2DATABASE"].as<std::string>();
        std::string command = "python3 " + path2comand + " " + path_server_database;
        processController.startProcess(name_server_database, command);
        server_database_active = true;
    }
}

void NodeManager::close_server_database()
{
    if (server_database_active)
    {
        YAML::Node config;
        try
        {
            std::string path_ = PATH;
            config = YAML::LoadFile(path_ + "server/src/node_manager/param/config.yaml");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error loading YAML file: " << e.what() << std::endl;
        }
        std::string name_server_database = config["NAME_DATABASE"].as<std::string>();
        processController.stopProcess(name_server_database);
        server_database_active = false;
    }
}

void NodeManager::start_robot()
{
#if !EN_CASA

    if (!start_robot_launch_file)
    {
        YAML::Node config;
        try
        {
            std::string path_ = PATH;
            config = YAML::LoadFile(path_ + "server/src/node_manager/param/config.yaml");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error loading YAML file: " << e.what() << std::endl;
        }
        std::string start_robot = config["START_ROBOT"].as<std::string>();
        std::string start_robot_name = config["NAME_START_ROBOT"].as<std::string>();
        std::string name_tf_service = config["NAME_TF_SERVICE"].as<std::string>();
        std::string tf_service = config["TF_SERVICE"].as<std::string>();

        pri1("Comienzo robot");
        processController.startProcess(start_robot_name, start_robot);
        processController.startProcess(name_tf_service, tf_service);
        start_robot_launch_file = true;

#if ROBOT
        create_subscription(Battery_Level);
#endif
    }
#endif
}

void NodeManager::stop_robot()
{
    if (!tf_service_client_)
    {
        tf_service_client_.reset();
        RCLCPP_INFO(node_manager->get_logger(), "Close Robot Pose Client destroy.");
    }

    if (start_robot_launch_file)
    {
#if ROBOT
        close_subscription(Battery_Level);
#endif
        YAML::Node config;
        try
        {
            std::string path_ = PATH;
            config = YAML::LoadFile(path_ + "server/src/node_manager/param/config.yaml");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error loading YAML file: " << e.what() << std::endl;
        }
        std::string name_robot = config["NAME_START_ROBOT"].as<std::string>();
        processController.stopProcess(name_robot);
        start_robot_launch_file = false;
    }
}

void NodeManager::start_bringup(std::string const &map_name)
{
#if !EN_CASA

    if (!bringup_launch_file)
    {
        YAML::Node config;
        std::string path_;
        try
        {
            path_ = PATH;
            config = YAML::LoadFile(path_ + "server/src/node_manager/param/config.yaml");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error loading YAML file: " << e.what() << std::endl;
        }
        std::string path_yaml = config["PATH2MAP"].as<std::string>();
        if (navegando)
        {
            std::string nav2_bringup_launch = config["NAV2_BRINGUP_LAUNCH"].as<std::string>();
            std::string name_nav2_bringup_launch = config["NAME_NAV2_BRINGUP_LAUNCH"].as<std::string>();

            std::string bringup = nav2_bringup_launch;
            bringup += path_ + path_yaml;
            bringup += "/" + map_name + ".yaml";
            pri1("++++++++++++++++++++++++++Start bring up GOAL POSE:" + bringup);
            processController.startProcess(name_nav2_bringup_launch, bringup);
        }
        else
        {
            processController.stopProcess(config["NAME_START_ROBOT"].as<std::string>());
            std::string nav_robot = config["NAVIGATION_ROBOT"].as<std::string>();
            std::string name_nav_robot = config["NAME_NAVIGATION_ROBOT"].as<std::string>();

            std::string bringup = nav_robot;
            bringup += path_ + path_yaml;
            bringup += "/" + map_name + ".yaml";
            pri1("+++++++++++++++++++Start person follow up GOAL POSE:" + bringup);
            processController.startProcess(name_nav_robot, bringup);

            std::string distance_tracker = config["DISTANCE_TRACKER"].as<std::string>();
            std::string name_distance_tracker = config["NAME_DISTANCE_TRACKER"].as<std::string>();
            processController.startProcess(name_distance_tracker, distance_tracker);

            std::string distance_controller = config["DISTANCE_CONTROLLER"].as<std::string>();
            std::string name_distance_controller = config["NAME_DISTANCE_CONTROLLER"].as<std::string>();
            processController.startProcess(name_distance_controller, distance_controller);

            std::string camara = config["CAMERA_CONTROLLER"].as<std::string>();
            std::string name_camara = config["NAME_CAMERA_CONTROLLER"].as<std::string>();
            processController.startProcess(name_camara, camara);

            std::string dinamixel = config["DYNAMIXEL"].as<std::string>();
            std::string name_dinamixel = config["NAME_DYNAMIXEL"].as<std::string>();
            processController.startProcess(name_dinamixel, dinamixel);


            std::string prueba = config["PRUEBA"].as<std::string>();
            std::string name_prueba = config["NAME_PRUEBA"].as<std::string>();
            processController.startProcess(name_prueba, prueba);
        }
        bringup_launch_file = true;
    }
#endif
}

void NodeManager::reset()
{
    pri1("Reset NodeManager");
#if !EN_CASA
    if (!tf_service_client_)
    {
        tf_service_client_.reset();
        RCLCPP_INFO(node_manager->get_logger(), "Close Robot Pose Client destroy.");
    }
    slam_launch_file = false;
    bringup_launch_file = false;
    start_robot_launch_file = false;
    processController.listProcesses();
    processController.stopAllProcesses();
#endif
}
