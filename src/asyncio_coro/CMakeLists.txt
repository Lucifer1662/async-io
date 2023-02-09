project ("asyncio")

file(GLOB SRC
     "src/**/*.cpp"
     "src/*.cpp"
     "src/**/*.cc"
     "src/*.cc"
)



add_library(asyncio_coro ${SRC}) 

if(MSVC)
set(CUSTOM_LIBS "-lwsock32 -lws2_32")
else()
set(CUSTOM_LIBS "-lssl -lcrypto")
endif()

target_link_libraries(asyncio_coro ${CUSTOM_LIBS})
target_include_directories(asyncio_coro PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>  
)
# target_include_directories(asyncio_coro INTERFACE ${CMAKE_SOURCE_DIR}/src/asyncio_coro/include )
target_compile_definitions(asyncio_coro PRIVATE)