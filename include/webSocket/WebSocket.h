#pragma once

#include <functional>
#include <string>

class WebSocket {
public:
    virtual ~WebSocket() = default;

    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void send(const std::string& message) = 0;

    using MessageCallBack = std::function<void(const std::string&)>;
    virtual void setMessageCallBack(MessageCallBack cb) = 0;

    virtual bool isConnected() const = 0;
};