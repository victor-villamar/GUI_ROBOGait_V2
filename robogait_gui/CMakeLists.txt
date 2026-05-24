cmake_minimum_required(VERSION 3.16)

project(robogait_gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(map_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(command_executor_msgs REQUIRED)
find_package(navigation_pkg REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    VirtualKeyboard
    Network
    Sql
    Widgets)

# Enable Qt's automatic MOC, UIC and RCC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set include directories
set(HEADERS
    include/Context/RobotContext.hpp
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
    include/Loader/MapFileLoader.hpp
    include/Loader/YamlLoader.hpp
    include/Map/Data/LaserScanData.hpp
    include/Map/Data/MapData.hpp
    include/Map/Data/ParticleCloudData.hpp
    include/Map/Data/PathData.hpp
    include/Map/Data/RobotPoseData.hpp
    include/Map/Interaction/Geometry/BezierSplineModel.hpp
    include/Map/Interaction/Geometry/SplineFitter.hpp
    include/Map/Interaction/Geometry/StrokeProcessor.hpp
    include/Map/Interaction/SplinePathEditor.hpp
    include/Map/Items/LaserLayerItem.hpp
    include/Map/Items/MapLayerItem.hpp
    include/Map/Items/ParticleCloudLayerItem.hpp
    include/Map/Items/PathLayerItem.hpp
    include/Map/Items/RobotLayerItem.hpp
    include/Map/Layer/LaserLayer.hpp
    include/Map/Layer/LayerInterface.hpp
    include/Map/Layer/MapLayer.hpp
    include/Map/Layer/ParticleCloudLayer.hpp
    include/Map/Layer/PathLayer.hpp
    include/Map/Layer/RobotLayer.hpp
    include/Map/Rendering/RenderCamera.hpp
    include/Map/Rendering/RenderPipeline.hpp
    include/Map/Rendering/RenderScene.hpp
    include/Map/Source/LaserSource.hpp
    include/Map/Source/MapSource.hpp
    include/Map/Source/ParticleCloudSource.hpp
    include/Map/Source/PathSource.hpp
    include/Map/Source/RobotPoseSource.hpp
    include/Map/Source/SourceInterface.hpp
    include/Map/Subscribers/LaserScanSubscriber.hpp
    include/Map/Subscribers/MapSubscriber.hpp
    include/Map/Subscribers/ParticleCloudSubscriber.hpp
    include/Map/Subscribers/PathSubscriber.hpp
    include/Map/Subscribers/TFSubscriber.hpp
    include/Map/Utils/PathSegmentation.hpp
    include/Map/Utils/Utils.hpp
    include/Map/MapVisualizationManager.hpp
    include/Perception/PersonDetectionMonitor.hpp
    include/Robot/ManualControl.hpp
    include/Robot/RobotDiscovery.hpp
    include/Robot/RobotManager.hpp
    include/Robot/RobotPlacementController.hpp
    include/Ros/Define.hpp
    include/Ros/QoSProfiles.hpp
    include/Ros/RosNodeManager.hpp
    include/Ros/TopicsName.hpp
    include/Services/RobotServiceBridge.hpp
    include/Services/RobotServiceClient.hpp
    include/Settings/DeveloperSettings.hpp
    include/Settings/UiSizingSettings.hpp
    include/Themes/AppTheme.hpp
    include/Themes/ThemeAuth.hpp
    include/Themes/ThemeBadges.hpp
    include/Themes/ThemeCore.hpp
    include/Themes/ThemeDialogsCommon.hpp
    include/Themes/ThemeHome.hpp
    include/Themes/ThemeJoystick.hpp
    include/Themes/ThemeMainMenu.hpp
    include/Themes/ThemeManualControl.hpp
    include/Themes/ThemeMap.hpp
    include/Themes/ThemeOrientationWheel.hpp
    include/Themes/ThemeRobotConnection.hpp
    include/Themes/ThemeSelection.hpp
    include/Themes/ThemeSettings.hpp
    include/User/Patient.hpp
    include/User/UserSession.hpp
)

# Set source files
set(SOURCES
    src/Context/RobotContext.cpp
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
    src/Loader/MapFileLoader.cpp
    src/Loader/YamlLoader.cpp
    src/Map/Data/LaserScanData.cpp
    src/Map/Data/MapData.cpp
    src/Map/Data/ParticleCloudData.cpp
    src/Map/Data/PathData.cpp
    src/Map/Data/RobotPoseData.cpp
    src/Map/Interaction/Geometry/BezierSplineModel.cpp
    src/Map/Interaction/Geometry/SplineFitter.cpp
    src/Map/Interaction/Geometry/StrokeProcessor.cpp
    src/Map/Interaction/SplinePathEditor.cpp
    src/Map/Items/LaserLayerItem.cpp
    src/Map/Items/MapLayerItem.cpp
    src/Map/Items/ParticleCloudLayerItem.cpp
    src/Map/Items/PathLayerItem.cpp
    src/Map/Items/RobotLayerItem.cpp
    src/Map/Layer/LaserLayer.cpp
    src/Map/Layer/MapLayer.cpp
    src/Map/Layer/ParticleCloudLayer.cpp
    src/Map/Layer/PathLayer.cpp
    src/Map/Layer/RobotLayer.cpp
    src/Map/Rendering/RenderCamera.cpp
    src/Map/Rendering/RenderPipeline.cpp
    src/Map/Rendering/RenderScene.cpp
    src/Map/Source/LaserSource.cpp
    src/Map/Source/MapSource.cpp
    src/Map/Source/ParticleCloudSource.cpp
    src/Map/Source/PathSource.cpp
    src/Map/Source/RobotPoseSource.cpp
    src/Map/Subscribers/LaserScanSubscriber.cpp
    src/Map/Subscribers/MapSubscriber.cpp
    src/Map/Subscribers/ParticleCloudSubscriber.cpp
    src/Map/Subscribers/PathSubscriber.cpp
    src/Map/Subscribers/TFSubscriber.cpp
    src/Map/Utils/PathSegmentation.cpp
    src/Map/Utils/Utils.cpp
    src/Map/MapVisualizationManager.cpp
    src/Perception/PersonDetectionMonitor.cpp
    src/Robot/ManualControl.cpp
    src/Robot/RobotDiscovery.cpp
    src/Robot/RobotManager.cpp
    src/Robot/RobotPlacementController.cpp
    src/Ros/Define.cpp
    src/Ros/QoSProfiles.cpp
    src/Ros/RosNodeManager.cpp
    src/Ros/TopicsName.cpp
    src/Services/RobotServiceBridge.cpp
    src/Services/RobotServiceClient.cpp
    src/Settings/DeveloperSettings.cpp
    src/Settings/UiSizingSettings.cpp
    src/Themes/AppTheme.cpp
    src/Themes/ThemeAuth.cpp
    src/Themes/ThemeBadges.cpp
    src/Themes/ThemeCore.cpp
    src/Themes/ThemeDialogsCommon.cpp
    src/Themes/ThemeHome.cpp
    src/Themes/ThemeJoystick.cpp
    src/Themes/ThemeMainMenu.cpp
    src/Themes/ThemeManualControl.cpp
    src/Themes/ThemeMap.cpp
    src/Themes/ThemeOrientationWheel.cpp
    src/Themes/ThemeRobotConnection.cpp
    src/Themes/ThemeSelection.cpp
    src/Themes/ThemeSettings.cpp
    src/User/Patient.cpp
    src/User/UserSession.cpp
    src/main.cpp
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
    yaml-cpp
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
    Qt6::Widgets
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
    ${std_msgs_INCLUDE_DIRS}
    ${nav_msgs_INCLUDE_DIRS}
    ${tf2_INCLUDE_DIRS}
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

option(ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wold-style-cast
        -Woverloaded-virtual
        -Wnon-virtual-dtor
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wnull-dereference
        -Wuseless-cast
        -Wdouble-promotion
        -Wformat=2
        -Wzero-as-null-pointer-constant
        -Wcast-qual
        -Wconversion
        -Wfloat-conversion
    )

    if(ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Werror
        )
    endif()
endif()

# Add dependencies
ament_target_dependencies(${PROJECT_NAME}
    rclcpp
    ament_index_cpp
    geometry_msgs
    nav_msgs
    map_msgs
    nav2_msgs
    tf2
    tf2_ros
    tf2_geometry_msgs
    std_srvs
    command_executor_msgs
    navigation_pkg
    yaml-cpp
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

install(DIRECTORY params
    DESTINATION share/${PROJECT_NAME}
)

ament_package()
