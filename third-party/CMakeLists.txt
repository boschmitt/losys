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
