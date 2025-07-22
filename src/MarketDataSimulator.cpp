#include "../include/MarketDataSimulator.h"
#include "../include/MarketDataMessage.h"
#include "../include/ThreadSafeMessageQueue.h"
#include "../include/OrderSide.h"
#include "../include/MarketDataGenerator.h"

#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>

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

void MarketDataSimulator::setSourceType(SourceType type) {
    sourceType_ = type;
}

vector<MarketDataMessage> MarketDataSimulator::loadFromFile(const string& filePath) {
    vector<MarketDataMessage> messages;

    ifstream file(filePath);
    if (!file.is_open()) throw runtime_error("Could not open file: " + filePath);

    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        string symbol, priceStr, quantityStr, sideStr, timestampStr;

        if (!getline(iss, symbol, ',')) continue;
        if (!getline(iss, priceStr, ',')) continue;
        if (!getline(iss, quantityStr, ',')) continue;
        if (!getline(iss, sideStr, ',')) continue;
        if (!getline(iss, timestampStr, ',')) continue;

        try {
            double price = stod(priceStr);
            int quantity = stoi(quantityStr);
            OrderSide side = from_string(sideStr);
            uint64_t rawTs = stoull(timestampStr);
            auto timestamp = chrono::steady_clock::time_point(chrono::nanoseconds(rawTs));

            messages.emplace_back(MarketDataMessage{symbol, price, quantity, side, timestamp});
        } 
        catch (const std::exception& e) {
            throw std::runtime_error("Error parsing CSV line: " + line + " - " + e.what());
        }
    }

    return messages;
}

void MarketDataSimulator::run() {
    vector<MarketDataMessage> messages;

    if (sourceType_ == SourceType::FILE) {
        if (filePath_.empty()) throw runtime_error("File path is not set for file source type.");
        messages = loadFromFile(filePath_);
    }
    else {
        MarketDataGeneratorConfig config{
            .symbols = {"AAPL", "GOOGL", "TSLA", "MSFT", "AMZN", "NFLX", "NVDA", "JPM"},
            .basePrice = 100.0,
            .priceVolatility = 5.0,
            .minQuantity = 1,
            .maxQuantity = 100,
            .numMessages = 100,
            .seed = 1 // Fixed seed for reproducibility
        };
        MarketDataGenerator generator(config);
        messages = generator.generate();
    }

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