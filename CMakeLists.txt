cmake_minimum_required(VERSION 3.21)

project(SegoRenderer
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(cmake/sdl2-config.cmake)
include(cmake/CopyFiles.cmake)


#engine
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SegEngine)

add_subdirectory(sandbox)
