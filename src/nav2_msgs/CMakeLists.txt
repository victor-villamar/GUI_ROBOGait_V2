cmake_minimum_required(VERSION 3.20)

project(nav2_msgs VERSION 1.3.12)

if(NOT "$ENV{ROS_DISTRO}" STREQUAL "lyrical")
    message(FATAL_ERROR
        "The local nav2_msgs compatibility package must only be built with ROS 2 Lyrical "
        "Humble and Jazzy builds must use the nav2_msgs package provided by /opt/ros")
endif()

find_package(ament_cmake REQUIRED)
find_package(action_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

set(msg_files
    "msg/Particle.msg"
    "msg/ParticleCloud.msg"
)

# Keep these definitions aligned with the Nav2 Jazzy wire API. The package is
# intentionally generated with Lyrical tooling so a Lyrical GUI can exchange
# these actions with a Jazzy robot.
set(action_files
    "action/ComputePathToPose.action"
    "action/FollowPath.action"
    "action/NavigateToPose.action"
)

rosidl_generate_interfaces(${PROJECT_NAME}
    ${msg_files}
    ${action_files}
    DEPENDENCIES
        action_msgs
        builtin_interfaces
        geometry_msgs
        nav_msgs
        std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
