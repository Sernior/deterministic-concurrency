include("../cmake/GoogleTest.cmake")

add_executable(dsl_test test.cpp scenario1DThread.h)

target_compile_features(dsl_test PUBLIC cxx_std_20)

target_link_libraries(dsl_test gtest_main dc)
