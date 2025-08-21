#pragma once

#include "WebSocket.h"
#include <ixwebsocket/IXWebSocket.h>

#include <functional>
#include <string>
#include <memory>

class IxWebSocketClient : public WebSocket {
private:
    std::string url_;
    std::unique_ptr<ix::WebSocket> ws_;
    MessageCallBack messageCallBack_;  

public:
    explicit IxWebSocketClient(const std::string& url);
    ~IxWebSocketClient() override;

    void connect() override;
    void disconnect() override;
    void send(const std::string& message) override;

    void setMessageCallBack(MessageCallBack cb) override;

    bool isConnected() const override;
};