cmake_minimum_required (VERSION 2.8)
project (evt2root)

#if user does not specify prefix we assign it to the exec directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	message("Install Prefix not specified.")
	file(MAKE_DIRECTORY exec)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/exec CACHE PATH
			"Install Prefix" FORCE)
endif ()
message("Installing to ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package (ROOT REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

include_directories(ExpEvent/include)
add_subdirectory (ExpEvent)
set (EXTRA_LIBS ExpEventStatic)

add_subdirectory (src)

