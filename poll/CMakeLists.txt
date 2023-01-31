project ("asyncio")

file(GLOB SRC
     "*.cpp"
     "*.cc"
)

add_executable(asyncio ${SRC}) 

target_link_libraries(asyncio wsock32 ws2_32)