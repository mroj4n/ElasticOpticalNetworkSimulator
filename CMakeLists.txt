cmake_minimum_required(VERSION 3.10)
project(cProj)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Include additional settings
include(${CMAKE_CURRENT_SOURCE_DIR}/include.cmake)


# Define your main executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})