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
project(launcher_manager)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(logs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(interface_srv REQUIRED)

#Include directories
include_directories(
    include
    include/common
    include/process
)

# Add source files
set(SOURCES
    src/common/functions.cpp
    src/process/Process.cpp
    src/utils/YamlLoader.cpp
    src/LauncherManager.cpp
    src/ProcessController.cpp
    src/StateMachine.cpp
)

# Add dependencies
set(DEPENDENCIES
    ament_index_cpp
    rclcpp
    logs
    interface_srv
)

# Create executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${SOURCES}
)

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

add_executable(test_yaml_loader
    test/test_yaml_loader.cpp
    src/utils/YamlLoader.cpp
    src/common/functions.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    yaml-cpp
    ${Boost_LIBRARIES}
)

# target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)


target_link_libraries(test_process
    ${Boost_LIBRARIES}
)

target_link_libraries(test_process_controller
    ${Boost_LIBRARIES}
)

target_link_libraries(test_yaml_loader
    yaml-cpp
)

# Link dependencies
ament_target_dependencies(${PROJECT_NAME}
    ${DEPENDENCIES}
)

ament_target_dependencies(test_process
    ${DEPENDENCIES}
)

ament_target_dependencies(test_process_controller
    ${DEPENDENCIES}
)

ament_target_dependencies(test_yaml_loader
    ${DEPENDENCIES}
)

# Install executable
install(TARGETS ${PROJECT_NAME}
     DESTINATION lib/${PROJECT_NAME}
)

# Install tests
install(TARGETS test_process
     DESTINATION lib/${PROJECT_NAME}/test
)

install(TARGETS test_process_controller
     DESTINATION lib/${PROJECT_NAME}/test
)

install(TARGETS test_yaml_loader
     DESTINATION lib/${PROJECT_NAME}/test
)

# Install config file

if(HOME)
    message(STATUS "HOME Flag is defined. Installing config_casa.yaml as config.yaml")
    install(FILES "${CMAKE_SOURCE_DIR}/config/config_casa.yaml"
        DESTINATION share/${PROJECT_NAME}/config
        RENAME config.yaml
    )

elseif(ROBOT)
    message(STATUS "ROBOT Flag is defined. Installing config_robot.yaml as config.yaml")
    install(FILES "${CMAKE_SOURCE_DIR}/config/config_robot.yaml"
        DESTINATION share/${PROJECT_NAME}/config
        RENAME config.yaml
    )

elseif(UNI)
    message(STATUS "UNI Flag is defined. Installing config_uni.yaml as config.yaml")
    install(FILES "${CMAKE_SOURCE_DIR}/config/config_uni.yaml"
        DESTINATION share/${PROJECT_NAME}/config
        RENAME config.yaml
    )

else()
    message(STATUS "Installing default config.yaml")
    install(FILES "${CMAKE_SOURCE_DIR}/config/config.yaml"
        DESTINATION share/${PROJECT_NAME}/config/
    )
endif()

ament_package()
