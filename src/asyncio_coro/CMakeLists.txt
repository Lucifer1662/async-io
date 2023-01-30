project ("asyncio")

file(GLOB SRC
     "**/*.cpp"
     "*.cpp"
     "**/*.cc"
     "*.cc"
)

add_library(asyncio_coro ${SRC}) 

target_link_libraries(asyncio_coro wsock32 ws2_32)
target_include_directories(asyncio_coro INTERFACE ${CMAKE_SOURCE_DIR}/src )
target_compile_definitions(asyncio_coro PRIVATE)