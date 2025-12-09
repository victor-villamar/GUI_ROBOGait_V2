cmake_minimum_required(VERSION 3.8)

# Compile options
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project name
project(node_manager)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Add source files
set(SOURCES
    src/NodeManager.cpp
    src/ProcessController.cpp
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${SOURCES}
)

ament_target_dependencies(${PROJECT_NAME} rclcpp)

target_link_libraries(node_manager yaml-cpp ${Boost_INCLUDE_DIRS})

target_compile_features(node_manager PUBLIC cxx_std_20)

# Install executable
install(TARGETS ${PROJECT_NAME}
     DESTINATION lib/${PROJECT_NAME}
)

ament_package()
