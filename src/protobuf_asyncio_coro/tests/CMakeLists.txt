project ("asyncio")

set(CMAKE_CXX_STANDARD 20)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
     "*.cc"
)
include(GoogleTest)

enable_testing()

include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})


add_executable(protobuf_asyncio_coro_tests ${SRC})

if(MSVC)
set(CUSTOM_LIBS "-lwsock32 -lws2_32")
else()
set(CUSTOM_LIBS "-lssl -lcrypto")
endif()

target_link_libraries(protobuf_asyncio_coro_tests PUBLIC GTest::gtest_main asyncio_coro protobuf_asyncio_coro ${CUSTOM_LIBS} 
${PROTOBUF_LIBRARY}
)


gtest_discover_tests(protobuf_asyncio_coro_tests)

add_test(NAME protobuf_asyncio_coro_tests COMMAND protobuf_asyncio_coro_tests)
