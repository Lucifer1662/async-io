project ("asyncio")

file(GLOB SRC
     "**/*.cpp"
     "*.cpp"
     "**/*.cc"
     "*.cc"
)

add_library(protobuf_asyncio_coro ${SRC}) 

if(MSVC)
set(CUSTOM_LIBS "-lwsock32 -lws2_32")
else()
set(CUSTOM_LIBS "-lssl -lcrypto")
endif()

target_link_libraries(protobuf_asyncio_coro ${CUSTOM_LIBS} )
target_include_directories(protobuf_asyncio_coro INTERFACE ${CMAKE_SOURCE_DIR}/src )
target_compile_definitions(protobuf_asyncio_coro PRIVATE)