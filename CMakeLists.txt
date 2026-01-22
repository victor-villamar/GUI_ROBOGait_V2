cmake_minimum_required(VERSION 3.16)

project(robogait_gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network)

# Enable Qt's automatic MOC, UIC and RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set include directories
set(HEADERS
    include/Define.hpp
    include/RobotDiscovery.hpp
    include/RobotManager.hpp
    include/RosNodeManager.hpp
    include/TopicsName.hpp
)

# Set source files
set(SOURCES
    src/main.cpp
    src/RobotDiscovery.cpp
    src/RosNodeManager.cpp
    src/RobotManager.cpp
)

# Set resource files
set(RESOURCES
    qml.qrc
    qmlresources.qrc
)

# Set library dependencies
set(LIBS
    rclcpp::rclcpp
    ${geometry_msgs_LIBRARIES}
    ${OpenCV_LIBS}
)

# Set Qt library dependencies
set(QT_LIBS
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Network
)

# Add Qt resources
qt_add_resources(SOURCES
    ${RESOURCES}
)

# Include directories
include_directories(
    include

    ${rclcpp_INCLUDE_DIRS}
    ${geometry_msgs_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Qml_INCLUDE_DIRS}
    ${Qt6Quick_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
)

# Add executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Add dependencies
ament_target_dependencies(${PROJECT_NAME}
    rclcpp
    geometry_msgs
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${LIBS}
    ${QT_LIBS}
)

# Install
install(TARGETS ${PROJECT_NAME}
    DESTINATION ${PROJECT_NAME}/bin
)

ament_package()
