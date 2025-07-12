cmake_minimum_required(VERSION 3.14)
project(DMHandler)

set(CMAKE_CXX_STANDARD 17)

#--------Add Google Test via FetchContent--------
include(FetchContent)

set(FETCHCONTENT_DOWNLOAD_EXTRACT_TIMESTAMP TRUE CACHE BOOL "")

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

target_include_directories(market_data_feed
    PRIVATE ${PROJECT_SOURCE_DIR}/include 
)   

#------- Unit Tests -------
add_executable(test_simulator
    tests/tests_simulator.cpp 
    src/MarketDataSimulator.cpp
)

add_executable(test_thread_safe_message_queue
    tests/tests_thread_safe_message_queue.cpp
)

target_include_directories(test_simulator 
    PRIVATE ${PROJECT_SOURCE_DIR}/include 
)

target_include_directories(test_thread_safe_message_queue
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

#-------- Link Libraries -------
target_link_libraries(test_simulator
    gtest_main
)

target_link_libraries(test_thread_safe_message_queue
    gtest_main 
)

include(GoogleTest)
gtest_discover_tests(test_simulator)
gtest_discover_tests(test_thread_safe_message_queue)
#---------------------------------