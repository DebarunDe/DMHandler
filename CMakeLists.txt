cmake_minimum_required(VERSION 3.14)
project(DMHandler)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

#--------Add Google Test via FetchContent--------
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Prevent GoogleTest from overrriding our compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()
#-------------------------------------------------

#------- Main App ---------
add_executable(market_data_feed
    src/main.cpp
    src/MarketDataSimulator.cpp
)

#------- Unit Tests -------
add_executable(test_simulator
    tests/tests_simulator.cpp 
    src/MarketDataSimulator.cpp
)

#-------- Link Libraries -------
target_link_libraries(test_simulator
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_simulator)