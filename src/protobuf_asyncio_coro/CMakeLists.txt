project ("asyncio")

add_library(protobuf_asyncio_coro INTERFACE) 

if(MSVC)
set(CUSTOM_LIBS "-lwsock32 -lws2_32")
else()
set(CUSTOM_LIBS "-lssl -lcrypto")
endif()

target_include_directories(protobuf_asyncio_coro INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>  
)

add_subdirectory("tests")
enable_testing()