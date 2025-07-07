cmake_minimum_required(VERSION 3.14)
project(DMHandler)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(market_data_feed
    src/main.cpp
    src/MarketDataSimulator.cpp
)