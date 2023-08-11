project ("asyncio")

add_library(coro async_task.cpp) 


target_link_libraries(coro ${CUSTOM_LIBS})
target_include_directories(coro INTERFACE ${CMAKE_SOURCE_DIR}/src )
# target_include_directories(coro PUBLIC
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#   $<INSTALL_INTERFACE:>  
# )

target_compile_definitions(coro PRIVATE)
