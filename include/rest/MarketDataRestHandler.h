#pragma once

#include "../testSubscribers/MarketStatsDataSubscriber.h"

#include "crow.h"
#include <memory>
#include <thread>
#include <atomic>
#include <string>

class MarketDataRestApi {
private:
    std::shared_ptr<MarketDataStatsTracker> statsTracker_;
    crow::SimpleApp app_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};

    void runServer(uint16_t port);

public:
    MarketDataRestApi(std::shared_ptr<MarketDataStatsTracker> statsTracker);
    ~MarketDataRestApi();

    // No copies allowed
    MarketDataRestApi(const MarketDataRestApi&) = delete;
    MarketDataRestApi& operator=(const MarketDataRestApi&) = delete;

    void start(uint16_t port = 18080);
    void stop();

};