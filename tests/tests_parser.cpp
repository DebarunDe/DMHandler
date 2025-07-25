#include <gtest/gtest.h>

#include "../include/MarketDataParser.h"
#include "../include/MarketDataMessage.h"
#include "../include/OrderSide.h"

#include <string>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <optional>

using namespace std;

TEST(MarketDataParserTest, ParsesValidMessage) {
    string line = "AAPL,BUY,150.0,100,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message.value().symbol, "AAPL");
    EXPECT_EQ(message.value().side, OrderSide::BUY);
    EXPECT_DOUBLE_EQ(message.value().price, 150.0);
    EXPECT_EQ(message.value().quantity, 100);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message.value().timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST(MarketDataParserTest, ParsesSellMessage) {
    string line = "GOOGL,SELL,2800.0,50,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message.value().symbol, "GOOGL");
    EXPECT_EQ(message.value().side, OrderSide::SELL);
    EXPECT_DOUBLE_EQ(message.value().price, 2800.0);
    EXPECT_EQ(message.value().quantity, 50);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message.value().timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST(MarketDataParserTest, HandlesInvalidFormat) {
    string line = "GOOGL,SELL,50,SELL,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesEmptyLine) {
    string line = "";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesMalformedTimestamp) {
    string line = "AAPL,BUY,150.0,100,not_a_timestamp";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesInvalidOrderSide) {
    string line = "AAPL,INVALID_SIDE,150.0,100,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesMissingFields) {
    string line = "AAPL,BUY,150.0,100"; // Missing timestamp
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesExtraFields) {
    string line = "AAPL,BUY,150.0,100,1633072800000000000,extra_field";
    auto message = MarketDataParser::parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message.value().symbol, "AAPL");
    EXPECT_EQ(message.value().side, OrderSide::BUY);
    EXPECT_DOUBLE_EQ(message.value().price, 150.0);
    EXPECT_EQ(message.value().quantity, 100);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message.value().timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST(MarketDataParserTest, HandlesNonNumericPrice) {
    string line = "AAPL,BUY,not_a_number,100,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesNonNumericQuantity) {
    string line = "AAPL,BUY,150.0,not_a_number,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesNegativeQuantity) {
    string line = "AAPL,BUY,150.0,-100,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message.value().quantity, -100);
}

TEST(MarketDataParserTest, HandlesZeroQuantity) {
    string line = "AAPL,BUY,150.0,0,1633072800000000000";
    auto message = MarketDataParser::parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message.value().quantity, 0);
}

TEST(MarketDataParserTest, HandlesLargeTimestamp) {
    string line = "AAPL,BUY,150.0,100,9999999999999999999"; // Very large timestamp
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

TEST(MarketDataParserTest, HandlesWhitespace) {
    string line = "   AAPL,   BUY,   150.0,   100,   1633072800000000000   ";
    auto message = MarketDataParser::parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message.value().symbol, "AAPL");
    EXPECT_EQ(message.value().side, OrderSide::BUY);
    EXPECT_DOUBLE_EQ(message.value().price, 150.0);
    EXPECT_EQ(message.value().quantity, 100);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message.value().timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST(MarketDataParserTest, HandlesWhitespaceForField) {
    string line = "   AAPL,   BUY,    ,   100,   1633072800000000000   ";
    auto message = MarketDataParser::parse(line);

    ASSERT_FALSE(message.has_value());
}

