cmake_minimum_required(VERSION 3.16)
project(ScreenCapture)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)  # Include current directory for headers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

set(SOURCES
    src/main.cpp
    src/frame_sender.cpp
    src/screen_capture.cpp
    src/overlay_window.cpp
    src/detections.cpp
    include/overlay_window.h
)

include_directories(include)

# Executable
add_executable(ScreenCapture ${SOURCES})

# Include directories (so headers are found)
target_include_directories(ScreenCapture PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(ScreenCapture
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${OpenCV_LIBS}
    CURL::libcurl
)