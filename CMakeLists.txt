cmake_minimum_required(VERSION 3.0)

include(CheckCXXCompilerFlag)

project(zhashmap)

set(CMAKE_CXX_STANDARD 17)

macro(add_compiler_flags)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGN}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGN}")
endmacro(add_compiler_flags)

check_cxx_compiler_flag("-pg" has_gprof "int main() { return 0; }")

#if (has_gprof)
#	add_compiler_flags(-pg)
#endif()

add_subdirectory(third_party/abseil-cpp)

include_directories(src)
include_directories(third_party/google)
include_directories(third_party/abseil-cpp)

add_executable(bench_map tests/bench_map.cc)
target_link_libraries(bench_map absl_hash absl_raw_hash_set)

add_executable(test_hashmap tests/test_hashmap.cc)
