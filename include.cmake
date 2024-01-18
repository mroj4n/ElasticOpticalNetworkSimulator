# include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/networkStructure)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)

# List of all CPP source files
set(PROJECT_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/builder.cpp
)

