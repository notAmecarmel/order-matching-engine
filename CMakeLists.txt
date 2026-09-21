cmake_minimum_required(VERSION 3.20)

project(OrderMatchingEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(order_engine
    src/main.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/ConcurrentMatchingEngine.cpp
    src/ApiServer.cpp
)


add_executable(benchmark
    src/Benchmark.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)


target_include_directories(order_engine
    PRIVATE
    include
    external
)


target_include_directories(benchmark
    PRIVATE
    include
)