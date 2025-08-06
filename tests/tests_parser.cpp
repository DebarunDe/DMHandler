#include <gtest/gtest.h>
#include "../include/parser/FileMarketDataParser.h"
#include "../include/MarketDataMessage.h"
#include "../include/OrderSide.h"

#include <string>
#include <chrono>

using namespace std;

class FileMarketDataParserTest : public ::testing::Test {
protected:
    // cppcheck-suppress unusedStructMember
    FileMarketDataParser parser;
};

TEST_F(FileMarketDataParserTest, ParsesValidMessage) {
    string line = "AAPL,BUY,150.0,100,1633072800000000000";
    auto message = parser.parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->symbol, "AAPL");
    EXPECT_EQ(message->side, OrderSide::BUY);
    EXPECT_DOUBLE_EQ(message->price, 150.0);
    EXPECT_EQ(message->quantity, 100);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message->timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST_F(FileMarketDataParserTest, ParsesSellMessage) {
    string line = "GOOGL,SELL,2800.0,50,1633072800000000000";
    auto message = parser.parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->symbol, "GOOGL");
    EXPECT_EQ(message->side, OrderSide::SELL);
    EXPECT_DOUBLE_EQ(message->price, 2800.0);
    EXPECT_EQ(message->quantity, 50);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message->timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST_F(FileMarketDataParserTest, HandlesInvalidFormat) {
    string line = "GOOGL,SELL,50,SELL,1633072800000000000";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesEmptyLine) {
    string line = "";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesMalformedTimestamp) {
    string line = "AAPL,BUY,150.0,100,not_a_timestamp";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesInvalidOrderSide) {
    string line = "AAPL,INVALID_SIDE,150.0,100,1633072800000000000";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesMissingFields) {
    string line = "AAPL,BUY,150.0,100";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesExtraFields) {
    string line = "AAPL,BUY,150.0,100,1633072800000000000,extra_field";
    auto message = parser.parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->symbol, "AAPL");
    EXPECT_EQ(message->side, OrderSide::BUY);
    EXPECT_DOUBLE_EQ(message->price, 150.0);
    EXPECT_EQ(message->quantity, 100);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message->timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST_F(FileMarketDataParserTest, HandlesNonNumericPrice) {
    string line = "AAPL,BUY,not_a_number,100,1633072800000000000";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesNonNumericQuantity) {
    string line = "AAPL,BUY,150.0,not_a_number,1633072800000000000";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesNegativeQuantity) {
    string line = "AAPL,BUY,150.0,-100,1633072800000000000";
    auto message = parser.parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->quantity, -100);
}

TEST_F(FileMarketDataParserTest, HandlesZeroQuantity) {
    string line = "AAPL,BUY,150.0,0,1633072800000000000";
    auto message = parser.parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->quantity, 0);
}

TEST_F(FileMarketDataParserTest, HandlesLargeTimestamp) {
    string line = "AAPL,BUY,150.0,100,9999999999999999999";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

TEST_F(FileMarketDataParserTest, HandlesWhitespace) {
    string line = "   AAPL,   BUY,   150.0,   100,   1633072800000000000   ";
    auto message = parser.parse(line);

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->symbol, "AAPL");
    EXPECT_EQ(message->side, OrderSide::BUY);
    EXPECT_DOUBLE_EQ(message->price, 150.0);
    EXPECT_EQ(message->quantity, 100);
    EXPECT_EQ(chrono::duration_cast<chrono::nanoseconds>(message->timestamp.time_since_epoch()).count(), 1633072800000000000);
}

TEST_F(FileMarketDataParserTest, HandlesWhitespaceForField) {
    string line = "   AAPL,   BUY,    ,   100,   1633072800000000000   ";
    auto message = parser.parse(line);
    ASSERT_FALSE(message.has_value());
}

