cmake_minimum_required(VERSION 3.20)
project(cc_impl CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CC_IMPL_SANITIZE "Build with ASan + UBSan" ON)
option(CC_IMPL_BUILD_BENCHMARKS "Build Google Benchmark targets" OFF)

if(CC_IMPL_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# Prevent overriding parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(CC_IMPL_BUILD_BENCHMARKS)
  FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/refs/heads/main.zip
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(benchmark)
endif()

enable_testing()

add_subdirectory(unique_ptr)