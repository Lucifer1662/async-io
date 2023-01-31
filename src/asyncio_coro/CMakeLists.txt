project ("asyncio")

file(GLOB SRC
     "**/*.cpp"
     "*.cpp"
     "**/*.cc"
     "*.cc"
)

add_library(asyncio_coro ${SRC}) 

if(MSVC)
set(CUSTOM_LIBS "-lwsock32 -lws2_32")
else()
set(CUSTOM_LIBS "")
endif()

target_link_libraries(asyncio_coro ${CUSTOM_LIBS} )
target_include_directories(asyncio_coro INTERFACE ${CMAKE_SOURCE_DIR}/src )
target_compile_definitions(asyncio_coro PRIVATE)