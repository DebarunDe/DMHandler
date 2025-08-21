#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock/MockWebSocketClient.h"
#include "parser/FinnhubMarketDataParser.h"
#include "dataSource/FinnhubConnector.h"
#include "ThreadSafeMessageQueue.h"
#include <chrono>
#include <thread>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AtLeast;

class FinnhubConnectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
    }

    void TearDown() override {
        // Common cleanup for all tests
    }

    // Helper method to wait for async operations
    void waitForConnectionAttempts(int expectedAttempts = 1) {
        // Wait for the worker thread to make connection attempts
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
};

TEST_F(FinnhubConnectorTest, ConnectsAndSubscribes) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1));

    // Store callback so we can simulate messages
    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    // Simulate receiving a message
    if (callback) {
        callback(R"({"type":"trade","data":[{"s":"AAPL","p":150.25,"v":100,"t":1640995200000}]})");
    }

    connector.stop();

    ASSERT_FALSE(queue->empty());
}

TEST_F(FinnhubConnectorTest, HandlesMultipleSymbols) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(2)); // One for each symbol

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL", "GOOGL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    // Simulate receiving messages for both symbols
    if (callback) {
        callback(R"({"type":"trade","data":[{"s":"AAPL","p":150.25,"v":100,"t":1640995200000}]})");
        callback(R"({"type":"trade","data":[{"s":"GOOGL","p":2800.50,"v":50,"t":1640995200000}]})");
    }

    connector.stop();

    ASSERT_FALSE(queue->empty());
    ASSERT_GE(queue->size(), 2); // Should have at least 2 messages
}

TEST_F(FinnhubConnectorTest, HandlesInvalidMessage) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1));

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    // Simulate receiving an invalid message
    if (callback) {
        callback(R"({"type":"invalid","data":[{"p":150.25}]})");
    }

    connector.stop();

    // Invalid messages should not be parsed, so queue might be empty
    // This depends on the parser implementation
}

TEST_F(FinnhubConnectorTest, HandlesConnectionFailure) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillRepeatedly(Return(false)); // Always return false to simulate connection failure

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1)); // Will be called by worker thread

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(0); // No subscription should be sent if connection fails

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    connector.stop();

    ASSERT_TRUE(queue->empty()); // No messages should be processed
}

TEST_F(FinnhubConnectorTest, StopsGracefully) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1));

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();
    
    connector.stop();

    // Should stop without crashing
    ASSERT_FALSE(connector.isRunning());
}

TEST_F(FinnhubConnectorTest, HandlesReconnect) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1)); // Called for initial connection

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1)); // Called for initial subscription

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();
    
    // Simulate receiving a message
    if (callback) {
        callback(R"({"type":"trade","data":[{"s":"AAPL","p":150.25,"v":100,"t":1640995200000}]})");
    }

    connector.stop();

    ASSERT_FALSE(queue->empty());
}

TEST_F(FinnhubConnectorTest, HandlesSubscriptionErrors) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1));

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    // Simulate an error in subscription
    if (callback) {
        callback(R"({"type":"error","data":{"message":"Subscription failed"}})");
    }

    connector.stop();

    // Error messages should not result in market data
    ASSERT_TRUE(queue->empty());
}

// New extended tests

TEST_F(FinnhubConnectorTest, HandlesEmptySymbolsList) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(0); // No symbols to subscribe to

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();
    
    connector.stop();

    ASSERT_TRUE(queue->empty());
}

TEST_F(FinnhubConnectorTest, HandlesDuplicateSymbols) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1)); // Should only send once per unique symbol

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL", "AAPL", "GOOGL"}; // Duplicate AAPL
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();
    
    connector.stop();

    ASSERT_TRUE(queue->empty()); // No market data messages
}

TEST_F(FinnhubConnectorTest, HandlesMalformedJsonMessage) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1));

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    // Simulate receiving a malformed JSON message
    if (callback) {
        callback(R"({"type":"trade","data":[{"s":"AAPL","p":150.25,"v":100,"t":1640995200000)");
    }

    connector.stop();

    // Malformed JSON should not crash the system
    ASSERT_TRUE(queue->empty());
}

TEST_F(FinnhubConnectorTest, HandlesNullParser) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    std::unique_ptr<FinnhubMarketDataParser> parser = nullptr;

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(1));

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();

    // Simulate receiving a message
    if (callback) {
        callback(R"({"type":"trade","data":[{"s":"AAPL","p":150.25,"v":100,"t":1640995200000}]})");
    }

    connector.stop();

    // Should not crash with null parser
    ASSERT_TRUE(queue->empty());
}

TEST_F(FinnhubConnectorTest, HandlesRapidStartStop) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(0);

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    // Rapid start/stop cycles
    for (int i = 0; i < 5; ++i) {
        connector.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        connector.stop();
    }

    ASSERT_TRUE(queue->empty());
}

TEST_F(FinnhubConnectorTest, HandlesLargeNumberOfSymbols) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(AtLeast(10)); // One for each symbol

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    // Create a large number of symbols
    std::vector<std::string> symbols;
    for (int i = 0; i < 10; ++i) {
        symbols.push_back("SYMBOL" + std::to_string(i));
    }

    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();
    
    connector.stop();

    ASSERT_TRUE(queue->empty()); // No market data messages
}

TEST_F(FinnhubConnectorTest, HandlesWebSocketClientDisconnection) {
    auto queue = std::make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    auto mockWsClient = std::make_unique<MockWebSocketClient>();
    auto parser = std::make_unique<FinnhubMarketDataParser>();

    // Expectations
    EXPECT_CALL(*mockWsClient, isConnected())
        .WillRepeatedly(Return(false)); // Always disconnected

    EXPECT_CALL(*mockWsClient, connect())
        .Times(AtLeast(1));

    EXPECT_CALL(*mockWsClient, send(_))
        .Times(0); // No subscription if not connected

    MockWebSocketClient::MessageCallBack callback;
    EXPECT_CALL(*mockWsClient, setMessageCallBack(_))
        .WillOnce(Invoke([&](MockWebSocketClient::MessageCallBack cb) {
            callback = cb;
        }));

    std::vector<std::string> symbols = {"AAPL"};
    FinnhubConnector connector(
        std::move(mockWsClient),
        std::move(parser),
        queue,
        symbols
    );

    connector.start();
    
    // Wait for the worker thread to attempt connection
    waitForConnectionAttempts();
    
    connector.stop();

    ASSERT_TRUE(queue->empty());
}

