cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
project (evt2root)

#if user does not specify prefix we assign it to the exec directory
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	message(STATUS "Install Prefix not specified.")
	file(MAKE_DIRECTORY exec)
	get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/exec REALPATH)
	set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH
			"Install Prefix" FORCE)
endif ()
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

#Define the default build type
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release CACHE STRING
		"Build type, options are: None Debug Release
			RelWithDebInfo MinSizeRel."
		FORCE)
	message(STATUS "Build type not defined, using default.")
ENDIF(NOT CMAKE_BUILD_TYPE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_definitions(-std=c++0x)

option(USE_EXPEVENT "Use custom sorting library" OFF)

find_package (ROOT REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

include_directories(utilities/include)
add_subdirectory(utilities)

include_directories(modules/include)
include_directories(buffers/include)
include_directories(include)
add_subdirectory(buffers)

add_subdirectory (modules)

if(${USE_EXPEVENT})
	include_directories(ExpEvent/include)
	add_subdirectory (ExpEvent)
	set(EXTRA_LIBS ExpEvent_Static)
endif()

set (EXTRA_LIBS ${EXTRA_LIBS} ModulesStatic Buffers Utilities)
add_subdirectory (src)

