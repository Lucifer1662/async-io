project ("asyncio")
set(CMAKE_CXX_STANDARD 17)

 
file(GLOB SRC
     "*.h"
     "*.cpp"
)

add_executable(hello_server ${SRC}) 

target_link_libraries(hello_server PUBLIC asyncio wsock32 ws2_32)

