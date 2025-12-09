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
project(logs)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# Create the logs library
add_library(${PROJECT_NAME} INTERFACE)

# Configurate include directories
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Configurate dependencies
ament_target_dependencies(${PROJECT_NAME} INTERFACE
  rclcpp
)

# Test executable
add_executable(test_logs test/test.cpp)
target_link_libraries(test_logs ${PROJECT_NAME})
ament_target_dependencies(test_logs rclcpp)

# Install the library and headers
install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(TARGETS test_logs
  DESTINATION lib/${PROJECT_NAME}
)

# Export the library so that other packages can use it
ament_export_include_directories(include)
ament_export_targets(export_${PROJECT_NAME})
ament_export_dependencies(rclcpp)

# Linting (optional)
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()