cmake_minimum_required(VERSION 2.6)

project(capsh)

# Always build with all warnings.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes")

include_directories(include)
add_subdirectory(src)
