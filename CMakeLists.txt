cmake_minimum_required(VERSION 3.14.0)

set(PROJ_NAME "Deterministic Concurrency")
project(${PROJ_NAME} VERSION 1.4.0 LANGUAGES CXX)

add_library(deterministic_concurrency INTERFACE)
target_include_directories(deterministic_concurrency INTERFACE include)
target_compile_features(deterministic_concurrency INTERFACE cxx_std_17)

add_library(Boost INTERFACE)
target_include_directories(deterministic_concurrency INTERFACE boost_1_83_0)
SET(CMAKE_C_COMPILER vc141)
option(DC_COMPILE_TESTS "Build the dsl tests" OFF)
option(DC_COMPILE_MAIN "Build the main.cpp" OFF)

if(DC_COMPILE_TESTS)
    add_subdirectory(tests)
endif()

#SET(BOOST_ROOT "C:/boost")

find_package(Boost REQUIRED COMPONENTS thread system)

if(DC_COMPILE_MAIN AND Boost_FOUND)
    set(SOURCE_LIST
    main.cpp)
    add_executable(test ${SOURCE_LIST})
    target_link_libraries(test deterministic_concurrency Boost::boost Boost::thread) 
endif()

#C:\Program Files\boost\boost-1.82.0/lib;
#C:\Program Files\boost\boost-1.82.0/stage/lib;
#C:/Program Files/boost/boost-1.82.0/lib;
#C:/Program Files/boost/boost-1.82.0/../lib;
#C:/Program Files/boost/boost-1.82.0/stage/lib;
#PATHS;C:/boost/lib;C:/boost;/sw/local/lib"