cmake_minimum_required(VERSION 3.14.0)

set(PROJ_NAME "Deterministic Concurrency")
project(${PROJ_NAME} VERSION 0.0.1 LANGUAGES CXX)

add_library(deterministic_concurrency INTERFACE)
target_include_directories(deterministic_concurrency INTERFACE include)
target_compile_features(deterministic_concurrency INTERFACE cxx_std_17)

option(DC_COMPILE_TESTS "Build the dsl tests" OFF)
option(DC_COMPILE_MAIN "Build the main.cpp" OFF)

if(DC_COMPILE_TESTS)
    add_subdirectory(tests)
endif()

if(DC_COMPILE_MAIN)
    set(SOURCE_LIST
    main.cpp)
    add_executable(test ${SOURCE_LIST})
    target_link_libraries(test PRIVATE deterministic_concurrency) 
endif()
