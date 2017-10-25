# Distributed under the BSD License (See accompanying file /LICENSE )
# CMake build : losys tool
cmake_minimum_required(VERSION 3.5)
project(losys VERSION 0.1 LANGUAGES C CXX)

# Default build type: Release
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Dependencies
# =============================================================================
add_subdirectory(third-party)

# Project soruce files
# =============================================================================
add_subdirectory(source)
