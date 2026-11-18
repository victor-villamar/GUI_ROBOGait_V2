cmake_minimum_required(VERSION 3.20)

project(command_executor_msgs)

# Find Packages
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# set messages files
set(msg_files
    "msg/RobotStatus.msg"
)

# set service files
set(srv_files
    "srv/Cmd.srv"
    "srv/GetMapData.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
    ${msg_files}
    ${srv_files}
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()