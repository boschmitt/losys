# Distributed under the BSD License (See accompanying file /LICENSE )
# CMake build : losys project

set(losys_src_files
  ${CMAKE_CURRENT_SOURCE_DIR}/base/collapse.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/opt/exorcism32.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xforms/esop_to_aig.cpp
  PARENT_SCOPE
  )

set(losys_bdd_src_files
  ${CMAKE_CURRENT_SOURCE_DIR}/bdd/collapse.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xforms/aig_to_bdd.cpp
  PARENT_SCOPE
  )

add_executable(losys main.cpp)
target_compile_features(losys PRIVATE cxx_auto_type)
target_link_libraries (losys PUBLIC libabc)
