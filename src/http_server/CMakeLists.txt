project ("asyncio")
set(CMAKE_CXX_STANDARD 17)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)

add_executable(server ${SRC})

target_link_libraries(server PUBLIC asyncio wsock32 ws2_32)

