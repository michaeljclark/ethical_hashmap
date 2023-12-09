cmake_minimum_required(VERSION 3.12)

project(ethical_hashmap)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include)

add_executable(bench_map tests/bench_map.cc)
add_executable(test_hash_map tests/test_hash_map.cc)
add_executable(test_hash_set tests/test_hash_set.cc tests/sha256.c)
add_executable(test_linked_hash_map tests/test_linked_hash_map.cc)
add_executable(test_linked_hash_set tests/test_linked_hash_set.cc tests/sha256.c)
add_executable(test_sha_delta tests/test_sha_delta.cc tests/sha256.c)
