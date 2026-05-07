cmake_minimum_required(VERSION 3.10)
project(SimpleFileConverter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all .cpp sources so moved files are discovered automatically
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/*.cpp
)

add_executable(converter ${SOURCES})

# Add project root and component dirs so includes like "parsers/..." resolve
target_include_directories(converter PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/parsers
    ${CMAKE_SOURCE_DIR}/detectors
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/builders
    ${CMAKE_SOURCE_DIR}/config
)

# Link filesystem library if needed (GCC < 9)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(converter stdc++fs)
endif()