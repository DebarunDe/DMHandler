#include "../include/MarketDataSimulator.h"
#include "../include/MarketDataMessage.h"
#include "../include/ThreadSafeMessageQueue.h"

#include <thread>
#include <chrono>
#include <vector>

using namespace std;

MarketDataSimulator::MarketDataSimulator(ThreadSafeMessageQueue<MarketDataMessage>& messageQueue):
    messageQueue_ ( messageQueue ),
    running_      ( false )
    { } 

void MarketDataSimulator::setReplayMode(ReplayMode mode, double factor) {
    replayMode_ = mode;
    replayFactor_ = factor;
};

void MarketDataSimulator::start() {
    if (running_) return;
    running_ = true;
    workerThread_ = std::thread(&MarketDataSimulator::run, this);
};

void MarketDataSimulator::stop() {
    if (!running_) return;
    running_ = false;
    if (workerThread_.joinable()) workerThread_.join();
};

void MarketDataSimulator::run() {
    auto now = chrono::steady_clock::now();

    vector<MarketDataMessage> messages = {
        {"AAPL", 150.0, 100, OrderSide::BUY,  now},
        {"AAPL", 150.5, 200, OrderSide::SELL, now + chrono::milliseconds(100)},
        {"GOOG", 2750.2, 50, OrderSide::BUY,  now + chrono::milliseconds(300)},
        {"TSLA", 690.3, 75, OrderSide::SELL,  now + chrono::milliseconds(450)}
    };

    auto simStart = messages.front().timestamp;
    auto realStart = chrono::steady_clock::now();

    for (const auto& msg : messages) {
        if (!running_) break;

        auto targetDelay = msg.timestamp - simStart;

        switch (replayMode_) {
            case ReplayMode::REALTIME:
                this_thread::sleep_until(realStart + targetDelay);
                break;
            case ReplayMode::ACCELERATED:
                this_thread::sleep_until(realStart + chrono::duration_cast<chrono::steady_clock::duration>(targetDelay * (1.0 / replayFactor_)));
                break;
            case ReplayMode::FIXED_DELAY:
                this_thread::sleep_for(chrono::milliseconds(static_cast<int>(100 / replayFactor_)));
                break;
        }

        MarketDataMessage emittedMessage = msg;
        emittedMessage.timestamp = chrono::steady_clock::now();
        messageQueue_.push(emittedMessage);
    }


}