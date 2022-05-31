cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME MatrixLED-Simulator)
project(${PROJECT_NAME})

file(GLOB TARGET_SRC src/*.hpp src/*.cpp)
add_executable(${PROJECT_NAME} ${TARGET_SRC})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBRARIES})

find_package(cppzmq)
target_link_libraries(${PROJECT_NAME} cppzmq)
