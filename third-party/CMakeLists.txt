# Distributed under the BSD License (See accompanying file /LICENSE )

# ABC
# TODO: Find another solution for this, for now I will keep creating patches
#       against Alan's repository. :(
# =============================================================================
if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/abc")
  find_package(Hg REQUIRED)
  message("Unable to find ABC, cloning...")
  execute_process(COMMAND hg clone https://bitbucket.org/alanmi/abc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  execute_process(COMMAND hg import --no-commit ../abc_01.diff
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/abc
    )
endif()

# add and exclude targets
add_subdirectory(abc)
set_target_properties(abc PROPERTIES EXCLUDE_FROM_ALL 1)

# TODO: find an alternative
# CUDD - The BDD library
# =============================================================================
include(ExternalProject)
ExternalProject_Add(
  cudd
  URL "http://msoeken.github.io/tools/cudd-3.0.0.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-obj --enable-static --enable-dddmp --prefix=${CMAKE_BINARY_DIR} --libdir=${CMAKE_BINARY_DIR}
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  # Logging
  LOG_CONFIGURE 1
  LOG_DOWNLOAD 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

# CLI
# =============================================================================
add_subdirectory(CLI11)
