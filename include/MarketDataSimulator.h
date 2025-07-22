#pragma once

#include "ThreadSafeMessageQueue.h"
#include "MarketDataMessage.h"

#include <string>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <filesystem>

enum class ReplayMode {
    REALTIME,
    ACCELERATED,
    FIXED_DELAY
};

enum class SourceType {
    FILE,
    GENERATED
};

class MarketDataSimulator {
private:
    static std::vector<MarketDataMessage> loadFromFile(const std::string& filePath);
    void run();

    SourceType sourceType_ = SourceType::FILE; // default to csv
    std::string filePath_ = "/Users/debarunde/VSCode/DMHandler/DMHandler/data/market_data.csv"; // Path to the CSV file if using file source

    ThreadSafeMessageQueue<MarketDataMessage>& messageQueue_;
    std::thread workerThread_;
    std::atomic<bool> running_;
    ReplayMode replayMode_ = ReplayMode::REALTIME;
    double replayFactor_ = 1.0; // Used for ACCELERATED mode

public:
    explicit MarketDataSimulator(ThreadSafeMessageQueue<MarketDataMessage>& messageQueue);

    void setSourceType(SourceType type);
    void setReplayMode(ReplayMode mode, double factor = 1.0);
    void start();
    void stop();

};