cmake_minimum_required(VERSION 3.16)
project(SmartGestureControls LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(app 
    src/main.cpp
    src/HandTracker.cpp
    src/GestureClassifier.cpp
    src/ActionMapper.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(app PRIVATE ${OpenCV_LIBS})
