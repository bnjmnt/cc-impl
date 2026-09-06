add_executable(test_unique_ptr test_unique_ptr.cc)
target_include_directories(test_unique_ptr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_unique_ptr PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_unique_ptr)

if(CC_IMPL_BUILD_BENCHMARKS)
  add_executable(bench_unique_ptr bench_unique_ptr.cc)
  target_include_directories(bench_unique_ptr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(bench_unique_ptr PRIVATE benchmark::benchmark)
endif()