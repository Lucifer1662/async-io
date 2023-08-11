
if(MSVC)
# add_subdirectory("asyncio_tests")
endif()
# add_subdirectory("asyncio_coro_tests")
enable_testing()
add_subdirectory("coro_tests")

