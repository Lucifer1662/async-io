project ("asyncio")
set(CMAKE_CXX_STANDARD 17)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)

add_executable(client ${SRC})

target_link_libraries(client PUBLIC asyncio wsock32 ws2_32)

