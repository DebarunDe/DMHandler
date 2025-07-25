#include "../include/MarketDataSimulator.h"
#include "../include/MarketDataMessage.h"
#include "../include/ThreadSafeMessageQueue.h"
#include "../include/OrderSide.h"
#include "../include/MarketDataGenerator.h"
#include "../include/MarketDataParser.h"

#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <optional>

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
        auto message = MarketDataParser::parse(line);

        if (message.has_value()) messages.emplace_back(message.value());
        else throw runtime_error("Failed to parse line: " + line);
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
        emittedMessage.timestamp = chrono::system_clock::now();
        messageQueue_.push(emittedMessage);
    }


}