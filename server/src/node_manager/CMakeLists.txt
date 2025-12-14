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
find_package(logs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

#Include directories
include_directories(
    include
    include/common
    include/process
)

# Add source files
set(SOURCES
    src/process/Process.cpp
    src/NodeManager.cpp
    src/ProcessController.cpp
)

set(DEPENDENCIES
    rclcpp
    logs
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${SOURCES}
)

ament_target_dependencies(${PROJECT_NAME}
    ${DEPENDENCIES}
)

target_link_libraries(${PROJECT_NAME}
    yaml-cpp
    ${Boost_LIBRARIES}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Test executables
add_executable(test_process
    test/test_process.cpp
    src/process/Process.cpp
)

add_executable(test_process_controller
    test/test_process_controller.cpp
    src/ProcessController.cpp
    src/process/Process.cpp
)

target_link_libraries(test_process
    ${Boost_LIBRARIES}
)

target_link_libraries(test_process_controller
    ${Boost_LIBRARIES}
)

ament_target_dependencies(test_process
    ${DEPENDENCIES}
)

ament_target_dependencies(test_process_controller
    ${DEPENDENCIES}
)

# Install executable
install(TARGETS ${PROJECT_NAME}
     DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS test_process
     DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS test_process_controller
     DESTINATION lib/${PROJECT_NAME}
)

ament_package()
