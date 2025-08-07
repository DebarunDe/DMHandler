#pragma once

#include "ThreadSafeMessageQueue.h"
#include "MarketDataMessage.h"

#include <string>
#include <vector>
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
    static std::vector<std::string> loadFromFile(const std::string& filePath);
    
    std::function<void(const std::string&)> fileSink_; // function to handle static file data
    std::function<void(const MarketDataMessage&)> generatedSink_; // function to handle simulated generated data

    std::string filePath_ = (std::filesystem::current_path() / "data/market_data.csv").string(); // Path to the CSV file if using file source
    SourceType sourceType_ = SourceType::FILE; // default to csv

    ThreadSafeMessageQueue<std::string>* fileMessageQueue_ = nullptr;
    ThreadSafeMessageQueue<MarketDataMessage>* generatedMessageQueue_ = nullptr;

    std::thread workerThread_;
    std::atomic<bool> running_;

    ReplayMode replayMode_ = ReplayMode::REALTIME;
    double replayFactor_ = 1.0; // Used for ACCELERATED mode

    void run();
    std::chrono::milliseconds getReplayDelay() const;
    std::chrono::steady_clock::duration getReplayOffset(
        const std::chrono::system_clock::time_point& simStart,
        const std::chrono::system_clock::time_point& msgTimeStamp
    ) const;

public:
    MarketDataSimulator(
        const std::function<void(const std::string&)>& fileSink,
        const std::function<void(const MarketDataMessage&)>& generatedSink,
        SourceType sourceType = SourceType::FILE
    );

    ~MarketDataSimulator();

    MarketDataSimulator(const MarketDataSimulator&) = delete;
    MarketDataSimulator& operator=(const MarketDataSimulator&) = delete;

    void setReplayMode(ReplayMode mode, double factor = 1.0);
    void start();
    void stop();
};