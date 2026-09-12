cmake_minimum_required(VERSION 3.24)
project(dovetail VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Multimedia Test)

qt_standard_project_setup()

set(DV_SOURCES
    src/main.cpp
    src/model/timeline_datamodel.cpp
    src/model/commands.cpp
    src/media/media_engine.cpp
)

qt_add_executable(dovetail ${DV_SOURCES})

target_include_directories(dovetail PRIVATE src)

target_link_libraries(dovetail PRIVATE
    Qt6::Core Qt6::Gui Qt6::Quick Qt6::Multimedia
)

qt_add_qml_module(dovetail
    URI Dovetail
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
    NO_RESOURCE_TARGET_PATH
)

enable_testing()
qt_add_executable(test_timeline tests/test_timeline.cpp
    src/model/timeline_datamodel.cpp src/model/commands.cpp)
target_include_directories(test_timeline PRIVATE src)
target_link_libraries(test_timeline PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME timeline COMMAND test_timeline)