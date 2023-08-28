# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.14)

project ("asyncio")

add_definitions( -DNOMINMAX )
enable_testing()

include_directories("/usr/include/c++/10")



if(MSVC)
add_compile_options("/fsanitize=address" "/MTd")
SET(CMAKE_CXX_FLAGS  "/await:strict")
else()

endif()

add_compile_options("-fcoroutines-ts")



include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
#
#FetchContent_Declare(googlebenchmark
#        GIT_REPOSITORY https://github.com/google/benchmark.git
#        GIT_TAG main) # need master for benchmark::benchmark
#
#FetchContent_MakeAvailable(googlebenchmark)





set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("build/")

add_subdirectory("src")
#add_subdirectory("benchmarks")



