#include <gtest/gtest.h>

#include "../include/testSubscribers/FileLoggerSubscriber.h"
#include "../include/MarketDataMessage.h"
#include "../utility/FilePathUtils.h"

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

TEST(FileLoggerSubscriber, LogsMarketDataMessage) {
    string filename = "tests/testlogs/filelogger/test_fileloggersubscriber.log";

    FileLoggerSubscriber fileLogger(filename);
    fileLogger.start();

    MarketDataMessage msg{
        .symbol = "AAPL",
        .side = OrderSide::BUY,
        .price = 150.0,
        .quantity = 100,
        .timestamp = chrono::system_clock::now()
    };

    fileLogger.onMarketData(msg);
    this_thread::sleep_for(100ms); // Allow for logging thread to process
    fileLogger.stop();

    // Read top line of log file to verify contents
    auto absolutePath = getProjectRoot() / filename;
    ifstream logFile(absolutePath.string());
    ASSERT_TRUE(logFile.is_open()) << "Log file could not be opened: " << filename;
    
    string line;
    getline(logFile, line);
    logFile.close();

    // Check if the line contains expected values
    stringstream expectedStream;
    auto timestamp = chrono::system_clock::to_time_t(msg.timestamp);
    expectedStream << std::put_time(std::localtime(&timestamp), "%m-%d-%Y %H:%M:%S") << " "
                    << msg.symbol << " "
                    << to_string(msg.side) << " "
                    << std::fixed << std::setprecision(2) << msg.price << " "
                    << "x" << msg.quantity;

    string expectedLine = expectedStream.str();
    EXPECT_TRUE(line.find("AAPL BUY 150.00 x100") != std::string::npos);
}
