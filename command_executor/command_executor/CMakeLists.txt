cmake_minimum_required(VERSION 3.16)

project(command_executor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake_ros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(yaml-cpp REQUIRED)
find_package(command_executor_msgs REQUIRED)

# Set include directories
include_directories(
    include
)

add_executable(${PROJECT_NAME}_node
    src/Functions.cpp
    src/CommandExecutor.cpp
    src/ProcessManager.cpp
    src/QoSProfile.cpp
    src/YamlLoader.cpp
    src/main.cpp
)

ament_target_dependencies(${PROJECT_NAME}_node
    rclcpp
    ament_index_cpp
    command_executor_msgs
)

target_include_directories(${PROJECT_NAME}_node PRIVATE
    include
)

target_link_libraries(${PROJECT_NAME}_node
    Boost::system
    Boost::filesystem
    yaml-cpp
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
  ament_add_ros_isolated_gtest(test_functions test/test_functions.cpp src/Functions.cpp)
  target_include_directories(test_functions PRIVATE include)
  target_link_libraries(test_functions
    Boost::system
    Boost::filesystem
  )

  # Test for YamlLoader
  ament_add_ros_isolated_gtest(test_yaml_loader test/test_yaml_loader.cpp src/YamlLoader.cpp)
  target_include_directories(test_yaml_loader PRIVATE include)
  target_link_libraries(test_yaml_loader
    yaml-cpp
  )
endif()

ament_package()
