#include <gtest/gtest.h>

#include "../include/testSubscribers/LoggingSubscriber.h"
#include "../include/MarketDataMessage.h"

#include <chrono>

using namespace std;

TEST(LoggingSubscriber, LogMessage) {
    LoggingSubscriber subscriber;

    MarketDataMessage msg{
        .symbol = "AAPL",
        .side = OrderSide::BUY,
        .price = 150.0,
        .quantity = 100,
        .timestamp = chrono::system_clock::now()
    };

    // Capture output
    testing::internal::CaptureStdout();
    
    subscriber.onMarketData(msg);
    string output = testing::internal::GetCapturedStdout();
    string expectedOutput = "[LOG] AAPL BUY @150.00 x100\n";
    EXPECT_EQ(output, expectedOutput);
}

