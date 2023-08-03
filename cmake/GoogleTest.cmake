set(CMAKE_BUILD_TYPE Release)

# Add FetchContent module (introduced in CMake 3.11)
include(FetchContent)

# Configure and download Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.13.0
)

FetchContent_MakeAvailable(googletest)
