#include "../../include/webSocket/IxWebSocketClient.h"

#include <iostream>
#include <chrono>
#include <stdexcept>
#include <thread>

using namespace std;

IxWebSocketClient::IxWebSocketClient(const std::string& url):
    url_(url),
    ws_(make_unique<ix::WebSocket>())
{
    ws_->setUrl(url_);
    ws_->setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message && messageCallBack_) messageCallBack_(msg->str);
        });
}

IxWebSocketClient::~IxWebSocketClient() { 
    ws_->stop();
    ws_.reset();
 }

void IxWebSocketClient::connect() {
    if (isConnected()) return;

    ws_->start();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for connection to establish
    // Wait until connected or timeout
    int attempts = 0;
    while (!isConnected() && attempts < 50) {
        this_thread::sleep_for(chrono::milliseconds(100));
        attempts++;
    }

    if (!isConnected()) throw runtime_error("Failed to connect to WebSocket: " + url_);
}

void IxWebSocketClient::disconnect() {
    ws_->stop();
}

void IxWebSocketClient::send(const std::string& message) {
    if (isConnected()) ws_->send(message);
    else throw runtime_error("WebSocket is not connected. Cannot send message.");
}

void IxWebSocketClient::setMessageCallBack(MessageCallBack cb) {
    messageCallBack_ = std::move(cb);
}

bool IxWebSocketClient::isConnected() const {
    return ws_ && ws_->getReadyState() == ix::ReadyState::Open;
}




