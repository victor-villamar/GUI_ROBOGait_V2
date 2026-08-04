cmake_minimum_required(VERSION 3.16)

project(command_executor)

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
  ament_add_gtest(test_functions test/test_functions.cpp src/Functions.cpp)
  target_include_directories(test_functions PRIVATE include)
  target_link_libraries(test_functions
    Boost::system
    Boost::filesystem
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
