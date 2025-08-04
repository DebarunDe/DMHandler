#include <gtest/gtest.h>

#include "../include/testSubscribers/MarketStatsDataSubscriber.h"

#include "../include/MarketDataMessage.h"

#include <chrono>

using namespace std;

TEST(MarketStatsDataSubscriber, UpdatesStatsCorrectly) {
    auto statsTracker = make_shared<MarketDataStatsTracker>();
    MarketDataStatsSubscriber subscriber(statsTracker);

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

    subscriber.onMarketData(msg1);
    subscriber.onMarketData(msg2);

    auto stats = statsTracker->getStats("AAPL");

    EXPECT_EQ(stats.lastPrice, 155.0);
    EXPECT_EQ(stats.totalVolume, 150);
    EXPECT_EQ(stats.tradeCount, 2);
    EXPECT_EQ(stats.highPrice, 155.0);
    EXPECT_EQ(stats.lowPrice, 150.0);
    EXPECT_DOUBLE_EQ(stats.getAveragePrice(), (150.0 * 100 + 155.0 * 50) / 150);
}

TEST(MarketStatsDataSubscriber, HandlesMultipleSymbols) {
    auto statsTracker = make_shared<MarketDataStatsTracker>();
    MarketDataStatsSubscriber subscriber(statsTracker);

    MarketDataMessage msg1{
        .symbol = "AAPL",
        .side = OrderSide::BUY,
        .price = 150.0,
        .quantity = 100,
        .timestamp = chrono::system_clock::now()
    };

    MarketDataMessage msg2{
        .symbol = "GOOGL",
        .side = OrderSide::SELL,
        .price = 2800.0,
        .quantity = 50,
        .timestamp = chrono::system_clock::now()
    };

    subscriber.onMarketData(msg1);
    subscriber.onMarketData(msg2);

    auto aaplStats = statsTracker->getStats("AAPL");
    auto googlStats = statsTracker->getStats("GOOGL");

    EXPECT_EQ(aaplStats.lastPrice, 150.0);
    EXPECT_EQ(aaplStats.totalVolume, 100);
    EXPECT_EQ(aaplStats.tradeCount, 1);
    
    EXPECT_EQ(googlStats.lastPrice, 2800.0);
    EXPECT_EQ(googlStats.totalVolume, 50);
    EXPECT_EQ(googlStats.tradeCount, 1);
}

TEST(MarketStatsDataSubscriber, HandlesEmptyStats) {
    auto statsTracker = make_shared<MarketDataStatsTracker>();
    MarketDataStatsSubscriber subscriber(statsTracker);

    auto stats = statsTracker->getStats("AAPL");

    EXPECT_EQ(stats.lastPrice, 0.0);
    EXPECT_EQ(stats.totalVolume, 0);
    EXPECT_EQ(stats.tradeCount, 0);
    EXPECT_EQ(stats.highPrice, numeric_limits<double>::lowest());
    EXPECT_EQ(stats.lowPrice, numeric_limits<double>::max());
    EXPECT_DOUBLE_EQ(stats.getAveragePrice(), 0.0);
}

TEST(MarketStatsDataSubscriber, UpdatesStatsWithSameSymbol) {
    auto statsTracker = make_shared<MarketDataStatsTracker>();
    MarketDataStatsSubscriber subscriber(statsTracker);

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

    subscriber.onMarketData(msg1);
    subscriber.onMarketData(msg2);

    auto stats = statsTracker->getStats("AAPL");

    EXPECT_EQ(stats.lastPrice, 155.0);
    EXPECT_EQ(stats.totalVolume, 150);
    EXPECT_EQ(stats.tradeCount, 2);

}

TEST(MarketStatsDataSubscriber, HandlesMultipleUpdates) {
    auto statsTracker = make_shared<MarketDataStatsTracker>();
    MarketDataStatsSubscriber subscriber(statsTracker);

    for (int i = 0; i < 10; ++i) {
        MarketDataMessage msg{
            .symbol = "JPM",
            .side = OrderSide::BUY,
            .price = 150.0 + i,
            .quantity = 10 + i,
            .timestamp = chrono::system_clock::now()
        };
        subscriber.onMarketData(msg);
    }

    auto stats = statsTracker->getStats("JPM");

    EXPECT_EQ(stats.lastPrice, 159.0);
    EXPECT_EQ(stats.totalVolume, 145);
    EXPECT_EQ(stats.tradeCount, 10);
    EXPECT_EQ(stats.highPrice, 159.0);
    EXPECT_EQ(stats.lowPrice, 150.0);
}