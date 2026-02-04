cmake_minimum_required(VERSION 3.16)

project(robogait_gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

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
    include/DataBase/Common/DbErrors.hpp
    include/DataBase/Common/DbTypes.hpp
    include/DataBase/Repository/ExperimentRepository.hpp
    include/DataBase/Repository/MapRepository.hpp
    include/DataBase/Repository/PatientRepository.hpp
    include/DataBase/Repository/Repository.hpp
    include/DataBase/Repository/UserRepository.hpp
    include/DataBase/DataBaseManager.hpp
    include/DataBase/RoboGaitDb.hpp
    include/Settings/DeveloperSettings.hpp
    include/Define.hpp
    include/Patient.hpp
    include/Robot/ManualControl.hpp
    include/Robot/RobotDiscovery.hpp
    include/Robot/RobotManager.hpp
    include/RosNodeManager.hpp
    include/TopicsName.hpp
    include/UserSession.hpp
)

# Set source files
set(SOURCES
    src/DataBase/Common/DbErrors.cpp
    src/DataBase/Common/DbTypes.cpp
    src/DataBase/Repository/ExperimentRepository.cpp
    src/DataBase/Repository/MapRepository.cpp
    src/DataBase/Repository/PatientRepository.cpp
    src/DataBase/Repository/Repository.cpp
    src/DataBase/Repository/UserRepository.cpp
    src/DataBase/DataBaseManager.cpp
    src/DataBase/RoboGaitDb.cpp
    src/Settings/DeveloperSettings.cpp
    src/main.cpp
    src/Patient.cpp
    src/Robot/ManualControl.cpp
    src/Robot/RobotDiscovery.cpp
    src/Robot/RobotManager.cpp
    src/RosNodeManager.cpp
    src/UserSession.cpp
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
    include/DataBase
    include/DataBase/Common
    include/DataBase/Repository

    ${rclcpp_INCLUDE_DIRS}
    ${geometry_msgs_INCLUDE_DIRS}
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
    geometry_msgs
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${LIBS}
    ${QT_LIBS}
)

# Install
install(TARGETS ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
)

install(FILES db_robogait.db
    DESTINATION share/${PROJECT_NAME}/database
)

ament_package()
