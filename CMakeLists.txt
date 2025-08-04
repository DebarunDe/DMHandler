cmake_minimum_required(VERSION 3.14)
project(DMHandler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Release or Debug" FORCE)

#--------Set Data file--------
configure_file(data/market_data.csv data/market_data.csv COPYONLY)

#--------Add Google Test via FetchContent--------
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Prevent GoogleTest from overrriding our compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

#--------Add curl--------
find_package(CURL REQUIRED)

#--------- Code Coverage Tools ---------
# Enable clang-tidy if available
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if (CLANG_TIDY_EXE) 
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# Enable cppcheck if available
find_program(CPPCHECK_EXE NAMES "cppcheck")
if(CPPCHECK_EXE)
    message(STATUS "Found cppcheck: ${CPPCHECK_EXE}")
    set(CMAKE_CXX_CPPCHECK
    "${CPPCHECK_EXE};--enable=all;--inconclusive;--quiet;--inline-suppr;--suppress=missingIncludeSystem;--std=c++17;--language=c++"
    )
endif()

option(CODE_COVERAGE "Enable code coverage" OFF)
if(CODE_COVERAGE)
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage -fprofile-update=atomic)
    link_libraries(gcov)
endif()

#-------------------------------------------------

#------- Main App ---------
add_executable(DMHandler
    src/main.cpp
    src/MarketDataSimulator.cpp
    src/MarketDataGenerator.cpp
    src/MarketDataParser.cpp
    src/MarketDataFeedHandler.cpp
    src/rest/MarketDataRestHandler.cpp
)

target_include_directories(DMHandler
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include 
        ${PROJECT_SOURCE_DIR}/third_party/crow/include
        ${PROJECT_SOURCE_DIR}/third_party/asio/asio/include
)   

target_link_libraries(DMHandler
    PRIVATE 
        pthread
        CURL::libcurl
)

#------- Unit Tests -------
add_executable(tests_simulator
    tests/tests_simulator.cpp 
    src/MarketDataSimulator.cpp
    src/MarketDataGenerator.cpp
    src/MarketDataParser.cpp
)

add_executable(tests_thread_safe_message_queue
    tests/tests_thread_safe_message_queue.cpp
)

add_executable(tests_generator
    tests/tests_generator.cpp
    src/MarketDataGenerator.cpp
)

add_executable(tests_parser
    tests/tests_parser.cpp
    src/MarketDataParser.cpp
)

add_executable(tests_feed_handler 
    tests/tests_feed_handler.cpp
    src/MarketDataFeedHandler.cpp
    src/MarketDataSimulator.cpp
    src/MarketDataGenerator.cpp
    src/MarketDataParser.cpp
    src/rest/MarketDataRestHandler.cpp
)

add_executable(tests_subscriber_loggingsubscriber 
    tests/tests_subscriber_loggingsubscriber.cpp
)

add_executable(tests_subscriber_fileloggersubscriber 
    tests/tests_subscriber_fileloggersubscriber.cpp
)

add_executable(tests_subscriber_statsdatasubscriber
    tests/tests_subscriber_statsdatasubscriber.cpp
)

add_executable(tests_marketdataresthandler
    tests/tests_marketdataresthandler.cpp
    src/rest/MarketDataRestHandler.cpp
)

target_include_directories(tests_simulator 
    PRIVATE ${PROJECT_SOURCE_DIR}/include 
)

target_include_directories(tests_thread_safe_message_queue
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(tests_generator
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(tests_parser 
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(tests_feed_handler
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/third_party/crow/include
        ${PROJECT_SOURCE_DIR}/third_party/asio/asio/include
)

target_include_directories(tests_subscriber_loggingsubscriber
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(tests_subscriber_fileloggersubscriber
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(tests_subscriber_statsdatasubscriber
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(tests_marketdataresthandler
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/third_party/crow/include
        ${PROJECT_SOURCE_DIR}/third_party/asio/asio/include
)


#-------- Link Libraries -------
target_link_libraries(tests_simulator
    gtest_main
)

target_link_libraries(tests_thread_safe_message_queue
    gtest_main 
)

target_link_libraries(tests_generator 
    gtest_main
)

target_link_libraries(tests_parser
    gtest_main
)

target_link_libraries(tests_feed_handler
    gtest_main
    CURL::libcurl
)

target_link_libraries(tests_subscriber_loggingsubscriber
    gtest_main
)

target_link_libraries(tests_subscriber_fileloggersubscriber
    gtest_main
)

target_link_libraries(tests_subscriber_statsdatasubscriber
    gtest_main
)

target_link_libraries(tests_marketdataresthandler
    gtest_main
    CURL::libcurl
)

include(GoogleTest)
gtest_discover_tests(tests_simulator)
gtest_discover_tests(tests_thread_safe_message_queue)
gtest_discover_tests(tests_generator)
gtest_discover_tests(tests_parser)
gtest_discover_tests(tests_feed_handler
    PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
gtest_discover_tests(tests_subscriber_loggingsubscriber)
gtest_discover_tests(tests_subscriber_fileloggersubscriber)
gtest_discover_tests(tests_subscriber_statsdatasubscriber)
gtest_discover_tests(tests_marketdataresthandler)
#---------------------------------