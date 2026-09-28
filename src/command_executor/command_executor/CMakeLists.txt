cmake_minimum_required(VERSION 3.16)

project(command_executor)

option(BUILD_TESTING "Build command_executor tests" OFF)

set(ROBOGAIT_ROS_DISTRO "$ENV{ROS_DISTRO}" CACHE STRING
    "ROS 2 distribution targeted by command_executor")
set_property(CACHE ROBOGAIT_ROS_DISTRO PROPERTY STRINGS humble jazzy lyrical)

string(TOLOWER "${ROBOGAIT_ROS_DISTRO}" ROBOGAIT_ROS_DISTRO_NORMALIZED)
set(ROBOGAIT_SUPPORTED_ROS_DISTROS humble jazzy lyrical)

if(NOT ROBOGAIT_ROS_DISTRO_NORMALIZED IN_LIST ROBOGAIT_SUPPORTED_ROS_DISTROS)
    message(FATAL_ERROR
        "Unsupported ROBOGAIT_ROS_DISTRO='${ROBOGAIT_ROS_DISTRO}'. "
        "Expected one of: humble, jazzy, lyrical")
endif()

string(TOUPPER "${ROBOGAIT_ROS_DISTRO_NORMALIZED}" ROBOGAIT_ROS_DISTRO_UPPER)
message(STATUS "Building command_executor for ROS 2 ${ROBOGAIT_ROS_DISTRO_NORMALIZED}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake_ros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(command_executor_msgs REQUIRED)

# Set include directories
include_directories(
    include
)

# Add library
add_library(${PROJECT_NAME}_lib STATIC
    src/Functions.cpp
    src/Define.cpp
    src/CommandExecutor.cpp
    src/ProcessManager.cpp
    src/QoSProfile.cpp
)

target_compile_definitions(${PROJECT_NAME}_lib PUBLIC
    "ROBOGAIT_ROS_${ROBOGAIT_ROS_DISTRO_UPPER}"
)

ament_target_dependencies(${PROJECT_NAME}_lib
    rclcpp
    command_executor_msgs
    sensor_msgs
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
    include
)

target_link_libraries(${PROJECT_NAME}_lib
    Boost::system
    Boost::filesystem
)

add_executable(${PROJECT_NAME}_node
    src/main.cpp
)

ament_target_dependencies(${PROJECT_NAME}_node
    rclcpp
)

target_link_libraries(${PROJECT_NAME}_node
    ${PROJECT_NAME}_lib
)

install(TARGETS ${PROJECT_NAME}_node
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
    DESTINATION include
)

install(DIRECTORY params/
    DESTINATION share/${PROJECT_NAME}/params
)

# Testing
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  # Test for Functions
  ament_add_gtest(test_functions test/test_functions.cpp)
  target_link_libraries(test_functions
    ${PROJECT_NAME}_lib
  )

  ament_add_gtest(test_process_manager
    test/test_process_manager.cpp
  )
  target_link_libraries(test_process_manager
    ${PROJECT_NAME}_lib
  )

  ament_add_ros_isolated_gtest(test_command_executor_ros test/test_command_executor_ros.cpp)
  target_include_directories(test_command_executor_ros PRIVATE include)
  ament_target_dependencies(test_command_executor_ros
    rclcpp
    command_executor_msgs
    sensor_msgs
  )
  target_link_libraries(test_command_executor_ros
    ${PROJECT_NAME}_lib
  )
endif()

ament_package()