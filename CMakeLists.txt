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

# Added libucl for linking
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lucl")


# Look for libraries under /usr/local, too:
include_directories("/usr/local/include")
link_directories("/usr/local/lib")

include_directories(include)
add_subdirectory(src)
