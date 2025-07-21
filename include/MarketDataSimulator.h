#pragma once

#include "ThreadSafeMessageQueue.h"
#include "MarketDataMessage.h"

#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>

enum class ReplayMode {
    REALTIME,
    ACCELERATED,
    FIXED_DELAY
};

class MarketDataSimulator {
private:
    void run();
    ThreadSafeMessageQueue<MarketDataMessage>& messageQueue_;
    std::thread workerThread_;
    std::atomic<bool> running_;
    ReplayMode replayMode_ = ReplayMode::REALTIME;
    double replayFactor_ = 1.0; // Used for ACCELERATED mode

public:
    explicit MarketDataSimulator(ThreadSafeMessageQueue<MarketDataMessage>& messageQueue);

    void setReplayMode(ReplayMode mode, double factor = 1.0);
    void start();
    void stop();

};