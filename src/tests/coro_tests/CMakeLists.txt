project ("asyncio")

set(CMAKE_CXX_STANDARD 20)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)
include(GoogleTest)

enable_testing()



add_executable(coro_tests ${SRC})


target_link_libraries(coro_tests PUBLIC GTest::gtest_main coro)


# target_link_libraries(run_tests PUBLIC bafka)
# target_compile_definitions(bafka PRIVATE)

gtest_discover_tests(coro_tests)

# add_test(NAME run_tests COMMAND run_tests)

add_test(NAME coro_tests COMMAND coro_tests)
