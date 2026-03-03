cmake_minimum_required(VERSION 3.16)

project(command_executor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(yaml-cpp REQUIRED)
find_package(command_executor_msgs REQUIRED)

# Set include directories
set(HEADERS
    include/CommandExecutor.hpp
    include/ProcessManager.hpp
    include/YamlLoader.hpp
)

include_directories(
    include
)

add_executable(${PROJECT_NAME}_node
    src/CommandExecutor.cpp
    src/ProcessManager.cpp
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

ament_package()
