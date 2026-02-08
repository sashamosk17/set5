cmake_minimum_required(VERSION 3.14)
project(HyperLogLog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hyperloglog
    src/main.cpp
    src/random_stream_gen.cpp
    src/hash_func_gen.cpp
    src/hyperloglog.cpp
    src/exact_counter.cpp
)

target_include_directories(hyperloglog PRIVATE src)