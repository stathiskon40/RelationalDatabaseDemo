# Minimum CMake version required
# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(RelationalDatabase VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type to Debug (optional, for debugging purposes)
set(CMAKE_BUILD_TYPE Debug)

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add executable
add_executable(RelationalDatabase ${SOURCES})

