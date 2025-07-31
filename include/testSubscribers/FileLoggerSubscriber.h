#pragma once

#include "../MarketDataSubscriber.h"
#include "../ThreadSafeMessageQueue.h"

#include "../utility/FilePathUtils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <stdexcept>

class MarketDataFileLogger : public IMarketDataSubscriber {
private:
    std::ofstream logFile_;
    std::string   filename_;
    std::mutex    fileMutex_; // Mutex to protect file access

    std::atomic<bool> running_ = false;
    std::thread       loggingThread_;
    ThreadSafeMessageQueue<MarketDataMessage> messageQueue_;

    void loggingLoop() {
        logFile_.open(filename_, std::ios::app);
        if (!logFile_.is_open()) throw std::runtime_error("Could not open log file: " + filename_);

        size_t flushCounter = 0;
        const size_t FLUSH_THRESHOLD = 10; 

        while (running_ || !messageQueue_.empty()) {
            MarketDataMessage msg;
            if (messageQueue_.tryPop(msg)) {
                std::lock_guard<std::mutex> lock(fileMutex_);
                auto timestamp = std::chrono::system_clock::to_time_t(msg.timestamp);
                logFile_ << std::put_time(std::localtime(&timestamp), "%m-%d-%Y %H:%M:%S") << " "
                    << msg.symbol << " "
                    << to_string(msg.side) << " "
                    << std::fixed << std::setprecision(2) << msg.price << " "
                    << "x" << msg.quantity << "\n";
                flushCounter++;
                if (flushCounter >= FLUSH_THRESHOLD) {
                    logFile_.flush();
                    flushCounter = 0;
                }
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep to avoid busy-waiting
            }
        }

        logFile_.flush(); // Ensure all remaining messages are written
        logFile_.close();
    }

public:
    explicit MarketDataFileLogger(const std::string& relativePath):
        filename_(relativePath)
        {
            if (relativePath.empty()) throw std::invalid_argument("Filename cannot be empty");
        
            auto absolutePath = getProjectRoot() / relativePath;
            // Create parent directory if needed
            std::filesystem::create_directories(absolutePath.parent_path());

            filename_ = absolutePath.string();
        }
    
    ~MarketDataFileLogger() {
        stop();
    }

    void start() {
        if (running_) return;
        running_ = true;
        loggingThread_ = std::thread(&MarketDataFileLogger::loggingLoop, this);
    }
    
    void stop() {
        if (!running_) return;
        running_ = false;
        if (loggingThread_.joinable()) loggingThread_.join();
    }

    void onMarketData(const MarketDataMessage& message) override {
        messageQueue_.push(message);
    }

};

class FileLoggerSubscriber : public IMarketDataSubscriber {
private:
    std::string filename_ = "logs/market_data.log"; //relative paths only
    MarketDataFileLogger fileLogger_;

public:
    FileLoggerSubscriber():
        fileLogger_(filename_)
        {
            fileLogger_.start();
        }
    
    FileLoggerSubscriber(const std::string& filename):
        filename_(filename),
        fileLogger_(filename)
        {
            fileLogger_.start();
        }
    
    ~FileLoggerSubscriber() {
        fileLogger_.stop();
    }
    
    void start() {
        fileLogger_.start();
    }

    void stop() {
        fileLogger_.stop();
    }

    void onMarketData(const MarketDataMessage& message) override {
        fileLogger_.onMarketData(message);
    }
};