cmake_minimum_required(VERSION 3.21)

project(SegoRenderer
    VERSION 0.1.0
    LANGUAGES C CXX
)
#set(CMAKE_VERBOSE_MAKEFILE on)

include(cmake/sdl2-config.cmake)
include(cmake/CopyFiles.cmake)
#engine


add_subdirectory(3rdL)
add_subdirectory(editor)
add_subdirectory(SegEngine)
add_subdirectory(sandbox)
#add_subdirectory(plugins)