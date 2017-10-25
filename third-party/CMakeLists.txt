# Distributed under the BSD License (See accompanying file /LICENSE )

# ABC
# =============================================================================
if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/abc")
  find_package(Hg REQUIRED)
  message("Unable to find ABC, cloning...")
  execute_process(COMMAND hg clone https://bitbucket.org/boschmitt/abc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# add and exclude targets
add_subdirectory(abc)
set_target_properties(abc PROPERTIES EXCLUDE_FROM_ALL 1)
