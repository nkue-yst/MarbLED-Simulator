cmake_minimum_required(VERSION 3.10)

### Setup C++ project ###
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### Setup main project ###
set(PROJECT_NAME MatrixLED-Simulator)
project(${PROJECT_NAME})

file(GLOB TARGET_SRC src/*.hpp src/*.cpp)
add_executable(${PROJECT_NAME} ${TARGET_SRC})
target_link_libraries(${PROJECT_NAME} libzmq)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

### Setup OpenCV ###
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBRARIES})

### Setup ZeroMQ ###
find_package(GnuTLS REQUIRED)
include_directories(${GNUTLS_INCLUDE_DIR})
add_subdirectory(thirdparty/libzmq)
add_subdirectory(thirdparty/cppzmq)
include_directories(thirdparty/libzmq/include)
include_directories(thirdparty/cppzmq)
