cmake_minimum_required(VERSION 3.19)

project(ControlStation VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

file(GLOB_RECURSE sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp src/*.h)

qt_add_executable(ControlStation res.qrc ${sources})

target_include_directories(ControlStation PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(ControlStation PRIVATE Qt6::Core Qt6::Widgets ${OpenCV_LIBS})

set_target_properties(ControlStation PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ControlStation
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
