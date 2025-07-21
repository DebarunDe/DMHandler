#include <gtest/gtest.h>
#include "../include/MarketDataGenerator.h"

#include <unordered_set>
#include <vector>
#include <string>

using namespace std;

TEST(MarketDataGeneratorTest, GeneratesCorrectAmountOfMessages) {
    MarketDataGeneratorConfig config {
        .symbols = {"AAPL", "GOOG"},
        .basePrice = 100.0,
        .priceVolatility = 5.0,
        .minQuantity = 10,
        .maxQuantity = 100,
        .numMessages = 50,
        .seed = 42 // Fixed seed for reproducibility
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();
    ASSERT_EQ(messages.size(), config.numMessages);
}

TEST(MarketDataGeneratorTest, GeneratedSymbolsAreValid) {
    vector<string> symbols = {"AAPL", "GOOG", "TSLA", "MSFT"};
    unordered_set<string> validSymbols(symbols.begin(), symbols.end());

    MarketDataGeneratorConfig config {
        .symbols = symbols,
        .basePrice = 100.0,
        .priceVolatility = 5.0,
        .minQuantity = 10,
        .maxQuantity = 100,
        .numMessages = 50,
        .seed = 42 // Fixed seed for reproducibility
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();

    for (const auto& msg : messages) ASSERT_TRUE(validSymbols.count(msg.symbol));
}

TEST(MarketDataGeneratorTest, PricesWithinExpectedRange) {
    double base = 200.0;
    double volatility = 10.0;

    MarketDataGeneratorConfig config{
        .symbols = {"NFLX"},
        .basePrice = base,
        .priceVolatility = volatility,
        .minQuantity = 10,
        .maxQuantity = 20,
        .numMessages = 100,
        .seed = 77
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();

    for (const auto& msg : messages) {
        EXPECT_GE(msg.price, base - volatility);
        EXPECT_LE(msg.price, base + volatility);
    }
}

TEST(MarketDataGeneratorTest, QuantityWithinRange) {
    double minQuantity = 50;
    double maxQuantity = 150;

    MarketDataGeneratorConfig config{
        .symbols = {"DIS"},
        .basePrice = 300.0,
        .priceVolatility = 20.0,
        .minQuantity = minQuantity,
        .maxQuantity = maxQuantity,
        .numMessages = 100,
        .seed = 99
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();

    for (const auto& msg : messages) {
        EXPECT_GE(msg.quantity, minQuantity);
        EXPECT_LE(msg.quantity, maxQuantity);
    }
}

TEST(MarketDataGeneratorTest, AlternatingTimestamps) {
    MarketDataGeneratorConfig config{
        .symbols = {"NVDA"},
        .basePrice = 120.0,
        .priceVolatility = 1.0,
        .minQuantity = 1,
        .maxQuantity = 1,
        .numMessages = 10,
        .seed = 1
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();

    for (size_t i = 1; i < messages.size(); ++i) {
        EXPECT_GT(messages[i].timestamp, messages[i - 1].timestamp);
    }
}

TEST(MarketDataGeneratorTest, ThrowsOnEmptySymbols) {
    MarketDataGeneratorConfig config{
        .symbols = {},
        .numMessages = 10,
        .seed = 42
    };

    EXPECT_THROW(MarketDataGenerator generator(config), std::invalid_argument);
}

TEST(MarketDataGeneratorTest, ThrowsOnZeroMessages) {
    MarketDataGeneratorConfig config{
        .symbols = {"AAPL"},
        .numMessages = 0,
        .seed = 42
    };

    EXPECT_THROW(MarketDataGenerator generator(config), std::invalid_argument);
}

TEST(MarketDataGeneratorTest, ThrowsOnNegativeVolatility) {
    MarketDataGeneratorConfig config{
        .symbols = {"GOOG"},
        .priceVolatility = -5.0,
        .numMessages = 5,
        .seed = 42
    };

    EXPECT_THROW(MarketDataGenerator generator(config), std::invalid_argument);
}

TEST(MarketDataGeneratorTest, ThrowsOnInvalidQuantities) {
    MarketDataGeneratorConfig config{
        .symbols = {"TSLA"},
        .minQuantity = 100,
        .maxQuantity = 50,
        .numMessages = 5,
        .seed = 42
    };

    EXPECT_THROW(MarketDataGenerator generator(config), std::invalid_argument);
}

TEST(MarketDataGeneratorTest, NoSeedStillGeneratesMessages) {
    MarketDataGeneratorConfig config{
        .symbols = {"GOOG"},
        .numMessages = 5,
        .seed = std::nullopt
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();

    EXPECT_EQ(messages.size(), config.numMessages);
    for (const auto& msg : messages) {
        EXPECT_EQ(msg.symbol, "GOOG");
    }
}

TEST(MarketDataGeneratorTest, HandlesExtremeVolatility) {
    MarketDataGeneratorConfig config{
        .symbols = {"TSLA"},
        .basePrice = 800.0,
        .priceVolatility = 500.0,
        .numMessages = 5,
        .seed = 42
    };

    MarketDataGenerator generator(config);
    auto messages = generator.generate();

    for (const auto& msg : messages) {
        EXPECT_GE(msg.price, 300.0);  // 800 - 500
        EXPECT_LE(msg.price, 1300.0); // 800 + 500
    }
}