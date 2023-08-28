project ("asyncio")

add_library(coro async_task.cpp callback_task.cpp) 


target_include_directories(coro INTERFACE ${CMAKE_SOURCE_DIR}/src )
target_compile_definitions(coro PRIVATE)
