cmake_minimum_required(VERSION 3.0)

include(CheckCXXCompilerFlag)

project(zhash_map)

set(CMAKE_CXX_STANDARD 17)

macro(add_compiler_flags)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGN}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGN}")
endmacro(add_compiler_flags)

check_cxx_compiler_flag("-pg" has_gprof "int main() { return 0; }")

#if (has_gprof)
#	add_compiler_flags(-pg)
#endif()

include_directories(src)
include_directories(tests)

add_executable(bench_map tests/bench_map.cc)

add_executable(test_hash_map tests/test_hash_map.cc)
add_executable(test_hash_set tests/test_hash_set.cc tests/sha256.c)
add_executable(test_linked_hash_map tests/test_linked_hash_map.cc)
add_executable(test_linked_hash_set tests/test_linked_hash_set.cc tests/sha256.c)
add_executable(test_sha_delta tests/test_sha_delta.cc tests/sha256.c)
