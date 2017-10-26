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

# Auxiliary
# =============================================================================
function(losys_target_name_for out file)
  get_filename_component(_extension ${_file} EXT)
  file(RELATIVE_PATH _relative ${PROJECT_SOURCE_DIR} ${file})
  string(REPLACE "${_extension}" "" _name ${_relative})
  string(REGEX REPLACE "/" "-" _name ${_name})
  set(${out} "${_name}" PARENT_SCOPE)
endfunction()

# Tests
# =============================================================================
set(third_party_include_dir "${PROJECT_SOURCE_DIR}/third-party")
set(losys_include_dirs ${PROJECT_SOURCE_DIR}/source)
set(losys_test_include_dirs ${losys_include_dirs} ${third_party_include_dir})

enable_testing()
add_custom_target(check
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Build and run all tests and benchmarks."
)

add_custom_target(tests COMMENT "Build all the tests.")
add_dependencies(check tests)

file(GLOB_RECURSE losys_tests "test/*.cpp")
foreach(_file IN LISTS losys_tests)
  losys_target_name_for(_target "${_file}")
  add_executable(${_target} EXCLUDE_FROM_ALL "${_file}" ${losys_src_files})
  add_dependencies(tests ${_target})
  target_compile_definitions(${_target} PUBLIC CATCH_CONFIG_MAIN)
  target_compile_features(${_target} PRIVATE cxx_auto_type)
  target_include_directories(${_target} PUBLIC ${losys_test_include_dirs})
  add_test(${_target} ${_target})
endforeach()
