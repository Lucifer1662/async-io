project ("asyncio")

file(GLOB SRC
     "**/*.cpp"
     "*.cpp"
     "**/*.cc"
     "*.cc"
)

add_library(asyncio ${SRC}) 

target_link_libraries(asyncio wsock32 ws2_32)
target_include_directories(asyncio INTERFACE ${CMAKE_SOURCE_DIR}/src )
target_compile_definitions(asyncio PRIVATE)