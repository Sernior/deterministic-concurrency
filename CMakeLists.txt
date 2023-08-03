cmake_minimum_required(VERSION 3.14.0)

set(PROJ_NAME "Deterministic Concurrency")
project(${PROJ_NAME} VERSION 0.0.0 LANGUAGES CXX)

add_library(dc INTERFACE)
target_include_directories(dc INTERFACE include)
target_compile_features(dc INTERFACE cxx_std_17)
    
    # Add include/ subdir to have VS show the header files.
#option(LIBTR_COMPILE_TESTS "Build the trlib tests" OFF)

option(DC_COMPILE_TESTS "Build the dsl tests" OFF)

#add_subdirectory(include)

#if(LIBTR_COMPILE_TESTS)
#    add_subdirectory(tests)
#endif()

if(DC_COMPILE_TESTS)
    add_subdirectory(tests)
endif()

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_LIST
    main.cpp)

add_executable(test ${SOURCE_LIST})
target_link_libraries(test PRIVATE dc) 