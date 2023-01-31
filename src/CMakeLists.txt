add_subdirectory("tests")

if(MSVC)
add_subdirectory("asyncio")
add_subdirectory("http_server")
add_subdirectory("http_client")
add_subdirectory("hello_server")
endif()

add_subdirectory("asyncio_coro")
enable_testing()

