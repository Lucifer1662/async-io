project ("particles")

file(GLOB SRC
    "*.cpp"
     "*.cc"
)
add_executable (benchmarks "${SRC}")

target_link_libraries(benchmarks benchmark::benchmark benchmark::benchmark_main)
target_link_libraries(benchmarks asyncio_coro)


# set(SFML_STATIC_LIBRARIES TRUE)
# target_link_libraries(app sfml-graphics sfml-window)


