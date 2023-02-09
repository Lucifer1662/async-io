project ("asyncio")

 

set(CMAKE_CXX_STANDARD 17)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)
include(GoogleTest)

enable_testing()



add_executable(asyncio_coro_tests ${SRC})

# target_link_libraries(run_tests bafka)


# SET(CMAKE_INSTALL_PREFIX "bafka" FORCE)

if(MSVC)
set(CUSTOM_LIBS "-lwsock32 -lws2_32")
else()
set(CUSTOM_LIBS "-lssl -lcrypto")
endif()

target_link_libraries(asyncio_coro_tests PUBLIC GTest::gtest_main asyncio_coro ${CUSTOM_LIBS} )


# target_link_libraries(run_tests PUBLIC bafka)
# target_compile_definitions(bafka PRIVATE)

gtest_discover_tests(asyncio_coro_tests)

# add_test(NAME run_tests COMMAND run_tests)

add_test(NAME asyncio_coro_tests COMMAND asyncio_coro_tests)
