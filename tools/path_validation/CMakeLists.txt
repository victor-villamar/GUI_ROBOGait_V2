cmake_minimum_required(VERSION 3.20)

project(path_validation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(robogait_gui REQUIRED)

add_executable(path_validation_processor
    src/path_processor.cpp
)

target_link_libraries(path_validation_processor PRIVATE
    robogait_gui::robogait_path_geometry
    Qt6::Core
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(path_validation_processor PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

install(TARGETS path_validation_processor
    DESTINATION lib/${PROJECT_NAME}
)

ament_package()
