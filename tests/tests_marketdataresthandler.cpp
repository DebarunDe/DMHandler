#include <gtest/gtest.h>

#include "../include/rest/MarketDataRestHandler.h"
#include "../include/MarketDataMessage.h"

#include "tests_helper.h"

#include <curl/curl.h>
#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include <limits>
#include <iostream>
#include <regex>
#include <cmath>

using namespace std;

class MarketDataRestHandlerTest : public ::testing::Test {
protected:
    shared_ptr<MarketDataStatsTracker> statsTracker;
    unique_ptr<MarketDataRestApi> restApi;

    void SetUp() override {
        statsTracker = make_shared<MarketDataStatsTracker>();
        restApi = make_unique<MarketDataRestApi>(statsTracker);
        restApi->start(18080); // Start the REST API on port 18080
        this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void TearDown() override {
        if (restApi) restApi->stop();
        restApi.reset();
        statsTracker.reset();
    }
};

TEST_F(MarketDataRestHandlerTest, GetStatsForSymbol) {
    // Simulate some market data messages
    MarketDataMessage msg1{
        .symbol = "AAPL",
        .side = OrderSide::BUY,
        .price = 150.0,
        .quantity = 100,
        .timestamp = chrono::system_clock::now()
    };
    
    MarketDataMessage msg2{
        .symbol = "AAPL",
        .side = OrderSide::SELL,
        .price = 155.0,
        .quantity = 50,
        .timestamp = chrono::system_clock::now()
    };
    
    statsTracker->update(msg1);
    statsTracker->update(msg2);
    
    // Make a GET request to the REST API
    string url = "http://localhost:18080/stats/AAPL";
    string response = httpGet(url);
    
    // Check if the response contains the expected data
    EXPECT_TRUE(fieldMatches(response, "averagePrice", 151.66666666666666));
    EXPECT_TRUE(fieldMatches(response, "lastPrice", 155.0));
    EXPECT_TRUE(fieldMatches(response, "lowPrice", 150.0));
    EXPECT_TRUE(fieldMatches(response, "highPrice", 155.0));
}

TEST_F(MarketDataRestHandlerTest, GetStatsForNonExistentSymbol) {
    // Make a GET request for a symbol that has no data
    string url = "http://localhost:18080/stats/NONEXISTENT";
    string response = httpGet(url);
    
    // Check if the response contains default values
    EXPECT_TRUE(fieldMatches(response, "averagePrice", 0));
    EXPECT_TRUE(fieldMatches(response, "lastPrice", 0));
    EXPECT_TRUE(fieldMatches(response, "tradeCount", 0));
    EXPECT_TRUE(fieldMatches(response, "totalVolume", 0));
}

TEST_F(MarketDataRestHandlerTest, HandlesUpdatedSymbol) {
    // Simulate some market data messages
    MarketDataMessage msg1{
        .symbol = "GOOGL",
        .side = OrderSide::BUY,
        .price = 2800.0,
        .quantity = 200,
        .timestamp = chrono::system_clock::now()
    };
    
    MarketDataMessage msg2{
        .symbol = "GOOGL",
        .side = OrderSide::SELL,
        .price = 2850.0,
        .quantity = 150,
        .timestamp = chrono::system_clock::now()
    };
    
    statsTracker->update(msg1);

    string url = "http://localhost:18080/stats/GOOGL";
    string response1 = httpGet(url);
    
    EXPECT_TRUE(fieldMatches(response1, "averagePrice", 2800.0));
    EXPECT_TRUE(fieldMatches(response1, "lastPrice", 2800.0));
    EXPECT_TRUE(fieldMatches(response1, "lowPrice", 2800.0));
    EXPECT_TRUE(fieldMatches(response1, "highPrice", 2800.0));

    statsTracker->update(msg2);
    this_thread::sleep_for(chrono::milliseconds(100)); // Allow some time for the stats to update
    
    string response2 = httpGet(url);
    
    cout << response2 << endl;
    EXPECT_TRUE(fieldMatches(response2, "averagePrice", 2821.4285714285716));
    EXPECT_TRUE(fieldMatches(response2, "lastPrice", 2850.0));
    EXPECT_TRUE(fieldMatches(response2, "lowPrice", 2800.0));
    EXPECT_TRUE(fieldMatches(response2, "highPrice", 2850.0));
}