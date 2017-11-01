# Distributed under the BSD License (See accompanying file /LICENSE )

set(tool_name "collapse")

add_executable(${tool_name} EXCLUDE_FROM_ALL
  main.cpp
  ${losys_src_files}
  ${losys_bdd_src_files}
  )

target_compile_features(${tool_name} PRIVATE cxx_auto_type cxx_uniform_initialization)

target_include_directories(${tool_name}
  PRIVATE
    ${losys_include_dirs}
    ${third_party_include_dir}
    ${CMAKE_BINARY_DIR}/include
  )

target_link_libraries(${tool_name}
  PUBLIC
    libabc
    ${CMAKE_BINARY_DIR}/libcudd.a
  PRIVATE
    CLI11
  )
