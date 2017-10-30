# Distributed under the BSD License (See accompanying file /LICENSE )

set(tool_name "exorcism")

add_executable(${tool_name} EXCLUDE_FROM_ALL main.cpp ${losys_src_files})
target_compile_features(${tool_name} PRIVATE cxx_auto_type cxx_uniform_initialization)
target_include_directories(${tool_name} PUBLIC ${losys_include_dirs})
target_link_libraries(${tool_name} PUBLIC libabc)
