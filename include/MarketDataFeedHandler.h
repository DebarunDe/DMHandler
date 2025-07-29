#pragma once

#include "ThreadSafeMessageQueue.h"
#include "OrderSide.h"
#include "MarketDataMessage.h"
#include "MarketDataSimulator.h"
#include "MarketDataSubscriber.h"
#include "MarketDataParser.h"


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
    ThreadSafeMessageQueue<MarketDataMessage>& messageQueue_;

    std::vector<std::shared_ptr<IMarketDataSubscriber>> subscribers_;
    std::mutex subscriberMutex_;

    std::thread dispatcherThread_;
    std::atomic<bool> running_;

    void dispatchLoop();

public:
    explicit MarketDataFeedHandler(ThreadSafeMessageQueue<MarketDataMessage>& messageQueue);
    ~MarketDataFeedHandler();

    void subscribe(std::shared_ptr<IMarketDataSubscriber> subscriber);
    void unsubscribe(std::shared_ptr<IMarketDataSubscriber> subscriber);

    void start();
    void stop();
};