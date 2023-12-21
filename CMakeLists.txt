cmake_minimum_required(VERSION 3.10)
project(cProj)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Include additional settings
include(${CMAKE_CURRENT_SOURCE_DIR}/include.cmake)

# Find FlatBuffers package
find_package(FlatBuffers REQUIRED)
find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
if(NOT FLATBUFFERS_FLATC_EXECUTABLE)
  message(FATAL_ERROR "flatc compiler not found")
endif()



# Compile FlatBuffers schemas
file(GLOB FBS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.fbs")
foreach(FBS_FILE ${FBS_FILES})
  get_filename_component(FBS_NAME ${FBS_FILE} NAME_WE)
  set(FBS_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/${FBS_NAME}_generated.h")
  add_custom_command(
    OUTPUT ${FBS_OUTPUT}
    COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE} -c -o "${CMAKE_CURRENT_BINARY_DIR}/include/" ${FBS_FILE}
    DEPENDS ${FBS_FILE}
    COMMENT "Building FlatBuffer schema ${FBS_NAME}"
  )
  list(APPEND GENERATED_FBS_HEADERS ${FBS_OUTPUT})
endforeach()

# Include generated FlatBuffers headers
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# Define your main executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE flatbuffers::flatbuffers)

# Add custom target for FlatBuffers
add_custom_target(GenerateFBS DEPENDS ${GENERATED_FBS_HEADERS})
add_dependencies(${PROJECT_NAME} GenerateFBS)
