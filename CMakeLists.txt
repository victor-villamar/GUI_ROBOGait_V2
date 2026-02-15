cmake_minimum_required(VERSION 3.16)

project(robogait_gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(map_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    VirtualKeyboard
    Network
    Sql)

# Enable Qt's automatic MOC, UIC and RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set include directories
set(HEADERS
    include/Core/RoboGaitApplication.hpp
    include/DataBase/Common/DbErrors.hpp
    include/DataBase/Common/DbTypes.hpp
    include/DataBase/Repository/ExperimentRepository.hpp
    include/DataBase/Repository/MapRepository.hpp
    include/DataBase/Repository/PatientRepository.hpp
    include/DataBase/Repository/Repository.hpp
    include/DataBase/Repository/UserRepository.hpp
    include/DataBase/DataBaseManager.hpp
    include/DataBase/DatabaseSchema.hpp
    include/DataBase/RoboGaitDb.hpp
    include/Loader/YamlLoader.hpp
    include/Settings/DeveloperSettings.hpp
    include/Robot/ManualControl.hpp
    include/Robot/RobotDiscovery.hpp
    include/Robot/RobotManager.hpp
    include/Ros/Define.hpp
    include/Ros/RosNodeManager.hpp
    include/Ros/TopicsName.hpp
    include/User/Patient.hpp
    include/User/UserSession.hpp
    include/Map/MapData.hpp
    include/Map/RobotPose.hpp
    include/Map/MapManager.hpp
    include/Map/MapImageProvider.hpp
    include/Map/Utils/Utils.hpp
)

# Set source files
set(SOURCES
    src/Core/RoboGaitApplication.cpp
    src/DataBase/Common/DbErrors.cpp
    src/DataBase/Common/DbTypes.cpp
    src/DataBase/Repository/ExperimentRepository.cpp
    src/DataBase/Repository/MapRepository.cpp
    src/DataBase/Repository/PatientRepository.cpp
    src/DataBase/Repository/Repository.cpp
    src/DataBase/Repository/UserRepository.cpp
    src/DataBase/DataBaseManager.cpp
    src/DataBase/DatabaseSchema.cpp
    src/DataBase/RoboGaitDb.cpp
    src/Loader/YamlLoader.cpp
    src/Settings/DeveloperSettings.cpp
    src/main.cpp
    src/Robot/ManualControl.cpp
    src/Robot/RobotDiscovery.cpp
    src/Robot/RobotManager.cpp
    src/Ros/RosNodeManager.cpp
    src/User/Patient.cpp
    src/User/UserSession.cpp
    src/Map/MapData.cpp
    src/Map/RobotPose.cpp
    src/Map/MapManager.cpp
    src/Map/MapImageProvider.cpp
    src/Map/Utils/Utils.cpp
)

# Set resource files
set(RESOURCES
    qml.qrc
    qmlresources.qrc
)

# Set library dependencies
set(LIBS
    rclcpp::rclcpp
    ament_index_cpp::ament_index_cpp
    yaml-cpp
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
    Qt6::Sql
    Qt6::VirtualKeyboard
)

# Add Qt resources
qt_add_resources(SOURCES
    ${RESOURCES}
)

# Include directories
include_directories(
    include
    include/Core
    include/DataBase
    include/DataBase/Common
    include/DataBase/Repository
    include/Loader
    include/Robot
    include/Ros
    include/Settings
    include/User
    include/map
    include/Map/utils

    ${rclcpp_INCLUDE_DIRS}
    ${geometry_msgs_INCLUDE_DIRS}
    ${std_msgs_INCLUDE_DIRS}
    ${nav_msgs_INCLUDE_DIRS}
    ${tf2_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Qml_INCLUDE_DIRS}
    ${Qt6Quick_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Sql_INCLUDE_DIRS}
    ${Qt6VirtualKeyboard_INCLUDE_DIRS}
)

# Add executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Add dependencies
ament_target_dependencies(${PROJECT_NAME}
    rclcpp
    ament_index_cpp
    std_msgs
    geometry_msgs
    nav_msgs
    map_msgs
    tf2
    tf2_ros
    tf2_geometry_msgs
    yaml-cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${LIBS}
    ${QT_LIBS}
)

# Add test_node executable
add_executable(test_node_node
    test/test_node/src/main.cpp
    test/test_node/src/TestNode.cpp
    test/test_node/include/TestNode.hpp
)

# Include directories for test_node
target_include_directories(test_node_node PRIVATE
    test/test_node/include
    include/Ros
    ${rclcpp_INCLUDE_DIRS}
    ${std_msgs_INCLUDE_DIRS}
)

# Add dependencies for test_node
ament_target_dependencies(test_node_node
    rclcpp
    std_msgs
)

# Install
install(TARGETS ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS test_node_node
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY params
    DESTINATION share/${PROJECT_NAME}
)

ament_package()
