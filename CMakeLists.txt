cmake_minimum_required (VERSION 2.8)
project (evt2root)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package (ROOT REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

include_directories(${evt2root_SOURCE_DIR}/MSUEvent)
add_subdirectory (MSUEvent)
set (EXTRA_LIBS MSUEventStatic)

add_subdirectory (src)

