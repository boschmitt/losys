# Distributed under the BSD License (See accompanying file /LICENSE )
# CMake build : losys project
add_executable(losys main.cpp)
target_compile_features(losys PRIVATE cxx_auto_type)
target_link_libraries (losys PUBLIC libabc)
