include("../cmake/GoogleTest.cmake")

add_executable(dsl_test test.cpp scenario1DScheduler.h scenario2DScheduler.h)

target_compile_features(dsl_test PUBLIC cxx_std_17)

target_link_libraries(dsl_test gtest_main deterministic_concurrency)
