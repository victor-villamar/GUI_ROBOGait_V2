cmake_minimum_required(VERSION 3.20)

project(navigation_pkg)

# Find Packages
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(sensor_msgs REQUIRED)

# set messages files
set(msg_files
    "msg/CameraDetection.msg"
    "msg/User.msg"
)

rosidl_generate_interfaces(${PROJECT_NAME}
    ${msg_files}

    DEPENDENCIES
    sensor_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
