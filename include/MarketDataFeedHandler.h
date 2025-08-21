#pragma once

#include "ThreadSafeMessageQueue.h"
#include "OrderSide.h"
#include "MarketDataMessage.h"
#include "MarketDataSimulator.h"
#include "MarketDataSubscriber.h"
#include "MarketDataStatsTracker.h"
#include "SymbolStats.h"


#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <optional>
#include <mutex>
#include <memory>

class MarketDataFeedHandler {
private:
    std::shared_ptr<ThreadSafeMessageQueue<MarketDataMessage>> messageQueue_;
    std::shared_ptr<MarketDataStatsTracker> statsTracker_;

    std::vector<std::shared_ptr<IMarketDataSubscriber>> subscribers_;
    std::mutex subscriberMutex_;

    std::thread dispatcherThread_;
    std::atomic<bool> running_;

    void dispatchLoop();

public:
    explicit MarketDataFeedHandler(std::shared_ptr<ThreadSafeMessageQueue<MarketDataMessage>> messageQueue);
    ~MarketDataFeedHandler();

    const std::shared_ptr<MarketDataStatsTracker>& getStatsTracker() const;

    void subscribe(std::shared_ptr<IMarketDataSubscriber> subscriber);
    void unsubscribe(std::shared_ptr<IMarketDataSubscriber> subscriber);

    void start();
    void stop();
};