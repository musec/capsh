cmake_minimum_required(VERSION 2.6)

project(capsh)

# Use C++11.
add_definitions("-std=c++11")

# Build with lots and lots of warnings.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")

# Disregard warnings about C++98 compatibility: use C++11 vigorously.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")

# Disregard warnings about padding to alignment: padding is fine.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-padded")

# Ignore vtable warnings that make a bit more work for the linker.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-weak-vtables")

# libpreopen may not be installed system-wide, but it may be available locally
# in the development environment.
set(LIBPREOPEN_PATH "" CACHE PATH "Prefix containing libpreopen")

find_path(
	LIBPREOPEN_HEADER
	"libpreopen.h"
	DOC "Directory containing libpreopen.h"
	PATHS
		"${LIBPREOPEN_PATH}/include"
		"/usr/local/include"
		"/usr/include"
)

if(LIBPREOPEN_HEADER)
	message(STATUS "Found libpreopen header in ${LIBPREOPEN_HEADER}")
	include_directories(${LIBPREOPEN_HEADER})
else()
	message(FATAL_ERROR
		"Failed to find libpreopen.h! "
		"Try setting -D LIBPREOPEN_PATH=/path/to/prefix")
endif()

find_library(
	LIBPREOPEN_LIB
	"preopen"
	DOC "Path to libpreopen shared library"
	PATHS "${LIBPREOPEN_PATH}/lib"
)

if(LIBPREOPEN_LIB)
	message(STATUS "Found libpreopen: ${LIBPREOPEN_LIB}")
else()
	message(FATAL_ERROR
		"Failed to find libpreopen! "
		"Try setting -D LIBPREOPEN_PATH=/path/to/prefix")
endif()

include_directories(include)
add_subdirectory(src)
