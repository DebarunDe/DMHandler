#pragma once
#include "../../include/webSocket/IxWebSocketClient.h"
#include <gmock/gmock.h>

class MockWebSocketClient : public IxWebSocketClient {
public:
    MockWebSocketClient() : IxWebSocketClient("ws://localhost:8080") {}
    ~MockWebSocketClient() = default;
    
    MOCK_METHOD(void, connect, (), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(void, send, (const std::string& message), (override));
    MOCK_METHOD(void, setMessageCallBack, (MessageCallBack callback), (override));
};