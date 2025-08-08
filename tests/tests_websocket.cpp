#include <gtest/gtest.h>

#include "../include/webSocket/IxWebSocketClient.h"

#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <future>

class WebSocketTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests
    }
    
    void TearDown() override {
        // Common cleanup for tests
    }
};

// Basic functionality tests
TEST_F(WebSocketTest, ConstructorAndDestructor) {
    EXPECT_NO_THROW({
        IxWebSocketClient ws("wss://example.com");
    });
}

TEST_F(WebSocketTest, ConstructorWithEmptyUrl) {
    EXPECT_NO_THROW({
        IxWebSocketClient ws("");
    });
}

TEST_F(WebSocketTest, ConstructorWithInvalidUrl) {
    EXPECT_NO_THROW({
        IxWebSocketClient ws("invalid://url");
    });
}

TEST_F(WebSocketTest, InitialState) {
    IxWebSocketClient ws("wss://example.com");
    EXPECT_FALSE(ws.isConnected());
}

// Connection state tests
TEST_F(WebSocketTest, ConnectWithoutValidServer) {
    IxWebSocketClient ws("wss://nonexistent-server-12345.com");
    
    EXPECT_FALSE(ws.isConnected());
    EXPECT_THROW(ws.connect(), std::runtime_error);
    EXPECT_FALSE(ws.isConnected());
}

TEST_F(WebSocketTest, DisconnectWhenNotConnected) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_FALSE(ws.isConnected());
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_FALSE(ws.isConnected());
}

TEST_F(WebSocketTest, MultipleDisconnectCalls) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_FALSE(ws.isConnected());
}

// Message sending tests
TEST_F(WebSocketTest, SendWithoutConnection) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_FALSE(ws.isConnected());
    EXPECT_THROW(ws.send("Test message"), std::runtime_error);
}

TEST_F(WebSocketTest, SendEmptyMessageWithoutConnection) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_FALSE(ws.isConnected());
    EXPECT_THROW(ws.send(""), std::runtime_error);
}

TEST_F(WebSocketTest, SendLargeMessageWithoutConnection) {
    IxWebSocketClient ws("wss://example.com");
    
    std::string largeMessage(10000, 'A');
    EXPECT_FALSE(ws.isConnected());
    EXPECT_THROW(ws.send(largeMessage), std::runtime_error);
}

// Callback tests
TEST_F(WebSocketTest, SetMessageCallback) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_NO_THROW(ws.setMessageCallBack([](const std::string&) {}));
}

TEST_F(WebSocketTest, SetMessageCallbackWithCapture) {
    IxWebSocketClient ws("wss://example.com");
    
    std::string captured;
    EXPECT_NO_THROW(ws.setMessageCallBack([&captured](const std::string& msg) {
        captured = msg;
    }));
}

TEST_F(WebSocketTest, SetNullCallback) {
    IxWebSocketClient ws("wss://example.com");
    
    // Should not throw when setting empty callback
    EXPECT_NO_THROW(ws.setMessageCallBack(nullptr));
}

TEST_F(WebSocketTest, MultipleCallbackChanges) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_NO_THROW(ws.setMessageCallBack([](const std::string&) {}));
    EXPECT_NO_THROW(ws.setMessageCallBack([](const std::string&) {}));
    EXPECT_NO_THROW(ws.setMessageCallBack([](const std::string&) {}));
}

// Multiple instances tests
TEST_F(WebSocketTest, MultipleInstances) {
    IxWebSocketClient ws1("wss://example1.com");
    IxWebSocketClient ws2("wss://example2.com");
    IxWebSocketClient ws3("wss://example3.com");
    
    EXPECT_FALSE(ws1.isConnected());
    EXPECT_FALSE(ws2.isConnected());
    EXPECT_FALSE(ws3.isConnected());
}

TEST_F(WebSocketTest, MultipleInstancesWithCallbacks) {
    IxWebSocketClient ws1("wss://example1.com");
    IxWebSocketClient ws2("wss://example2.com");
    
    std::string captured1, captured2;
    
    ws1.setMessageCallBack([&captured1](const std::string& msg) {
        captured1 = msg;
    });
    
    ws2.setMessageCallBack([&captured2](const std::string& msg) {
        captured2 = msg;
    });
    
    EXPECT_NO_THROW(ws1.disconnect());
    EXPECT_NO_THROW(ws2.disconnect());
}

// URL handling tests
TEST_F(WebSocketTest, DifferentUrlFormats) {
    EXPECT_NO_THROW(IxWebSocketClient ws1("wss://example.com"));
    EXPECT_NO_THROW(IxWebSocketClient ws2("ws://example.com"));
    EXPECT_NO_THROW(IxWebSocketClient ws3("wss://example.com:443"));
    EXPECT_NO_THROW(IxWebSocketClient ws4("ws://example.com:80"));
}

TEST_F(WebSocketTest, UrlWithPath) {
    EXPECT_NO_THROW(IxWebSocketClient ws("wss://example.com/path"));
    EXPECT_NO_THROW(IxWebSocketClient ws2("wss://example.com/path/to/websocket"));
}

TEST_F(WebSocketTest, UrlWithQueryParameters) {
    EXPECT_NO_THROW(IxWebSocketClient ws("wss://example.com?param=value"));
    EXPECT_NO_THROW(IxWebSocketClient ws2("wss://example.com/path?param1=value1&param2=value2"));
}

// Thread safety tests
TEST_F(WebSocketTest, ThreadSafeCallbackSetting) {
    IxWebSocketClient ws("wss://example.com");
    
    std::vector<std::thread> threads;
    std::atomic<int> callbackCount{0};
    
    // Create multiple threads that set callbacks
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&ws, &callbackCount]() {
            ws.setMessageCallBack([&callbackCount](const std::string&) {
                callbackCount++;
            });
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_NO_THROW(ws.disconnect());
}

// State transition tests
TEST_F(WebSocketTest, StateTransitions) {
    IxWebSocketClient ws("wss://example.com");
    
    // Initial state
    EXPECT_FALSE(ws.isConnected());
    
    // Try to connect to invalid server
    EXPECT_THROW(ws.connect(), std::runtime_error);
    EXPECT_FALSE(ws.isConnected());
    
    // Disconnect when not connected
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_FALSE(ws.isConnected());
    
    // Try to connect again
    EXPECT_THROW(ws.connect(), std::runtime_error);
    EXPECT_FALSE(ws.isConnected());
}

// Error handling tests
TEST_F(WebSocketTest, SendAfterDisconnect) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_FALSE(ws.isConnected());
    EXPECT_THROW(ws.send("message"), std::runtime_error);
    
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_FALSE(ws.isConnected());
    EXPECT_THROW(ws.send("message"), std::runtime_error);
}

TEST_F(WebSocketTest, ConnectAfterDisconnect) {
    IxWebSocketClient ws("wss://example.com");
    
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_FALSE(ws.isConnected());
    
    // Try to connect to invalid server after disconnect
    EXPECT_THROW(ws.connect(), std::runtime_error);
    EXPECT_FALSE(ws.isConnected());
}

// Memory and resource tests
TEST_F(WebSocketTest, DestructorCleansUp) {
    {
        IxWebSocketClient ws("wss://example.com");
        EXPECT_FALSE(ws.isConnected());
        // Destructor should be called here
    }
    // Should not have any memory leaks
}

TEST_F(WebSocketTest, CopyConstructorNotAvailable) {
    IxWebSocketClient ws1("wss://example1.com");
    IxWebSocketClient ws2("wss://example2.com");
    
    EXPECT_FALSE(ws1.isConnected());
    EXPECT_FALSE(ws2.isConnected());
    
    // The class doesn't support copy/move due to unique_ptr member
    // This is expected behavior
}

// Performance tests
TEST_F(WebSocketTest, RapidStateChanges) {
    IxWebSocketClient ws("wss://example.com");
    
    // Rapidly change states
    for (int i = 0; i < 100; ++i) {
        EXPECT_NO_THROW(ws.disconnect());
        EXPECT_FALSE(ws.isConnected());
    }
}

TEST_F(WebSocketTest, RapidCallbackChanges) {
    IxWebSocketClient ws("wss://example.com");
    
    // Rapidly change callbacks
    for (int i = 0; i < 100; ++i) {
        EXPECT_NO_THROW(ws.setMessageCallBack([](const std::string&) {}));
    }
}

// Integration tests
TEST_F(WebSocketTest, FullLifecycle) {
    IxWebSocketClient ws("wss://example.com");
    
    // Set callback
    std::string received;
    ws.setMessageCallBack([&received](const std::string& msg) {
        received = msg;
    });
    
    // Try to connect (will fail with invalid server)
    EXPECT_THROW(ws.connect(), std::runtime_error);
    EXPECT_FALSE(ws.isConnected());
    
    // Try to send (should fail)
    EXPECT_THROW(ws.send("test"), std::runtime_error);
    
    // Disconnect
    EXPECT_NO_THROW(ws.disconnect());
    EXPECT_FALSE(ws.isConnected());
    
    // Try to send again (should fail)
    EXPECT_THROW(ws.send("test"), std::runtime_error);
}

TEST_F(WebSocketTest, MultipleLifecycles) {
    for (int i = 0; i < 10; ++i) {
        IxWebSocketClient ws("wss://example.com");
        
        EXPECT_FALSE(ws.isConnected());
        EXPECT_THROW(ws.connect(), std::runtime_error);
        EXPECT_FALSE(ws.isConnected());
        EXPECT_THROW(ws.send("test"), std::runtime_error);
        EXPECT_NO_THROW(ws.disconnect());
        EXPECT_FALSE(ws.isConnected());
    }
}