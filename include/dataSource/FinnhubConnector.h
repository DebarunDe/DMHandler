#pragma once

#include "../MarketDataMessage.h"
#include "../webSocket/IxWebSocketClient.h"
#include "../parser/FinnhubMarketDataParser.h"
#include "../ThreadSafeMessageQueue.h"

#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <mutex>

class FinnhubConnector {
private:
    std::unique_ptr<IxWebSocketClient> wsClient_;
    std::unique_ptr<FinnhubMarketDataParser> parser_;
    ThreadSafeMessageQueue<MarketDataMessage>& messageQueue_;
    mutable std::mutex subscribedSymbolsMutex_;
    std::vector<std::string> subscribedSymbols_;
    std::thread workerThread_;
    std::atomic<bool> running_;
    std::atomic<bool> teardownRequested_ = false;


    void tryConnect();
    void sendSubscribe(const std::string& symbol);
    void onMessageReceived(const std::string& message);
    void ingestLoop();

public:
    FinnhubConnector(
        std::unique_ptr<IxWebSocketClient> wsClient, 
        std::unique_ptr<FinnhubMarketDataParser> parser, 
        ThreadSafeMessageQueue<MarketDataMessage>& messageQueue, 
        std::vector<std::string> symbols = {}
    );

    ~FinnhubConnector();

    void start();
    void stop();
    void subscribeToSymbol(const std::string& symbol);
    bool isRunning() const;
        
};