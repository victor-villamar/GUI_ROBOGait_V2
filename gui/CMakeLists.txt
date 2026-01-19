cmake_minimum_required(VERSION 3.16)

project(gui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON) # Para enlazar imágenes y recursos

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

qt_add_executable(appgui
    src/main.cpp
    src/StringHandler.cpp
    src/utils.cpp
    src/cliente.cpp
    src/ToJson.cpp
    src/MapInfo.cpp
    src/database.cpp
    src/utils_ddbb.cpp
    src/NetworkDDBB.cpp
    src/RobotDiscovery.cpp
    src/RosNodeManager.cpp
    src/RobotManager.cpp


    include/StringHandler.h
    include/utils.h
    include/cliente.h
    include/ToJson.h
    include/MapInfo.h
    include/database.h
    include/utils_ddbb.h
    include/NetworkDDBB.h
    include/RobotDiscovery.hpp
    include/RosNodeManager.hpp
    include/TopicsName.hpp
    include/RobotManager.hpp
    include/Define.hpp
)

target_include_directories(appgui PRIVATE
    include
    ${geometry_msgs_INCLUDE_DIRS}
)

qt_add_qml_module(appgui
    URI gui
    VERSION 1.0

    QML_FILES
    qml/main.qml
    qml/ApplicationFlow.qml
    qml/ApplicationFlowForm.ui.qml
    qml/Home.qml
    qml/HomeForm.ui.qml
    qml/RobotConnection.qml
    qml/RobotConnectionForm.ui.qml
    qml/RobotCmdVel.qml

    # qml/MainMenuForm.ui.qml
    # qml/MainMenu.qml
    # qml/MapPage.qml
    # qml/MapPageForm.ui.qml
    # qml/SelectMap.qml
    # qml/SelectMapForm.ui.qml

    # qml/SelectPatient.qml
    # qml/SelectPatientForm.ui.qml
    # qml/RegisterPage.qml
    # qml/RegisterPatient.qml
    # qml/RegisterPatientForm.ui.qml

    # qml/ManualControl.qml
    # qml/ManualControlForm.ui.qml
    # qml/CreateMap.qml
    # qml/CreateMapForm.ui.qml
    # qml/RegisterMap.qml
    # qml/RegisterMapForm.ui.qml

    qml/extras/CustomToolBarForm.ui.qml
    qml/extras/CustomToolBar.qml
    # qml/extras/Joystick.qml
    # qml/extras/JoystickForm.ui.qml
    # qml/extras/Map.qml
    # qml/extras/SavePage.qml
    # qml/extras/SavePageForm.ui.qml
    # qml/extras/OrientationCircle.qml
    # qml/extras/OrientationCircleForm.ui.qml
    qml/extras/ErrorRectangle.qml
    # qml/extras/BoxImages.qml
    # qml/extras/BoxImagesForm.ui.qml
    # qml/extras/Guest.qml
    # qml/extras/GuestForm.ui.qml
    # qml/extras/Login.qml
    # qml/extras/LoginForm.ui.qml
    # qml/extras/SignIn.qml
    # qml/extras/SignInForm.ui.qml
    qml/extras/InformationRectangle.qml
    qml/extras/ConfirmationDialog.qml
    qml/extras/ConfigurationMenu.qml
    qml/extras/CustomBottomBar.qml
    qml/extras/CustomBottomBarForm.ui.qml
    qml/extras/Battery.qml
    # qml/extras/DescriptionPatientForm.ui.qml
    # qml/extras/DescriptionMapForm.ui.qml
    # qml/extras/ImageText.qml
    # qml/extras/PopUpInfoForm.ui.qml

    RESOURCES
    # icon_app include .svg
    images/icon_app/check-solid.svg
    images/icon_app/circle-user-regular.svg
    images/icon_app/eraser-solid.svg
    images/icon_app/file-arrow-down-solid.svg
    images/icon_app/keyboard_left.svg
    images/icon_app/pencil-solid.svg
    images/icon_app/pen-to-square-regular.svg
    images/icon_app/saveButton.png
    images/icon_app/trash-solid.svg
    images/icon_app/plus-solid.svg
    images/icon_app/circle-info-solid.svg
    images/icon_app/circle-question.svg
    images/icon_app/xmark-solid.svg
    images/icon_app/home.svg
    images/icon_app/manual_control.png

    # images sponsors
    images/sponsors/car.png
    images/sponsors/etsidi.png
    images/sponsors/ministerio.png
    images/sponsors/upm.png

    # images robot
    images/robot/robot.png
    images/robot/robot_with_arrow.png
    images/robot/robot_goal.png
    images/robot/robot_with_arrow_goal.png

    # images main menu
    images/menu/image_test.png
    images/menu/manual_control.png
    images/menu/select_map.png
    images/menu/select_patient.png

    # select action in Test
    images/action/draw_pencil.png
    images/action/goal.png

    # images
    images/my_map.png
    images/logo.png
    images/icon.png

    images/stop.png
    images/stop_light.png
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appgui PROPERTIES
    #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appgui
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
)

target_link_libraries(appgui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Network
    rclcpp::rclcpp
    ${geometry_msgs_LIBRARIES}
    ${OpenCV_LIBS}
)

# if(TARGET geometry_msgs::geometry_msgs__rosidl_generator_cpp)
#   target_link_libraries(appgui PRIVATE
#     geometry_msgs::geometry_msgs__rosidl_generator_cpp
#   )
# endif()

# if (ANDROID)
#     # Define las rutas de los íconos
#     set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android_icons")

#     # Especifica el ícono de la aplicación
#     set(ANDROID_APK_ICON "@mipmap/ic_launcher")

#     # Incluye recursos adicionales
#     qt_add_android_apk(android_apk_target
#         PACKAGE_SOURCE_DIR ${ANDROID_PACKAGE_SOURCE_DIR}
#     )
# endif()

include(GNUInstallDirs)
install(TARGETS appgui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY qml DESTINATION ${CMAKE_INSTALL_BINDIR})

ament_package()
