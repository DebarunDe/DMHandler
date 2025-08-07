#include "../include/MarketDataSimulator.h"
#include "../include/MarketDataMessage.h"
#include "../include/OrderSide.h"
#include "../include/MarketDataGenerator.h"


#include <fstream>
#include <stdexcept>
#include <sstream>
#include <thread>

using namespace std;

MarketDataSimulator::MarketDataSimulator(
    const function<void(const string&)>& fileSink,
    const function<void(const MarketDataMessage&)>& generatedSink,
    SourceType sourceType
):
    fileSink_(fileSink),
    generatedSink_(generatedSink),
    sourceType_(sourceType),
    running_(false)
{ }

MarketDataSimulator::~MarketDataSimulator() {
    stop();
}

void MarketDataSimulator::setReplayMode(ReplayMode mode, double factor) {
    replayMode_ = mode;
    replayFactor_ = factor;
};

void MarketDataSimulator::start() {
    if (running_) return;
    running_ = true;
    workerThread_ = thread(&MarketDataSimulator::run, this);
};

void MarketDataSimulator::stop() {
    if (!running_) return;
    running_ = false;
    if (workerThread_.joinable()) workerThread_.join();
};

vector<string> MarketDataSimulator::loadFromFile(const string& filePath) {
    vector<string> messages;

    ifstream file(filePath);
    if (!file.is_open()) throw runtime_error("Could not open file: " + filePath);

    string line;
    while (getline(file, line)) messages.emplace_back(line);

    return messages;
}

chrono::milliseconds MarketDataSimulator::getReplayDelay() const {
    switch (replayMode_) {
        case ReplayMode::REALTIME:
            return chrono::milliseconds(10);
        case ReplayMode::ACCELERATED:
        case ReplayMode::FIXED_DELAY:
            return chrono::milliseconds(static_cast<int>(10 / replayFactor_));
    }
    return chrono::milliseconds(10);
}

chrono::steady_clock::duration MarketDataSimulator::getReplayOffset(
    const chrono::system_clock::time_point& simStart,
    const chrono::system_clock::time_point& msgTimeStamp
) const {
    auto simDelta = msgTimeStamp - simStart;
    return chrono::duration_cast<chrono::steady_clock::duration>(simDelta * (1.0 / replayFactor_));
}

void MarketDataSimulator::run() {

    if (sourceType_ == SourceType::FILE) {
        auto messages = loadFromFile(filePath_);

        for (const auto& rawLine : messages) {
            if (!running_) break;
            fileSink_(rawLine);
            this_thread::sleep_for(getReplayDelay());
        }
    } else {
        MarketDataGeneratorConfig config{
            .symbols = {"AAPL", "GOOGL", "TSLA", "MSFT", "AMZN", "NFLX", "NVDA", "JPM"},
            .basePrice = 100.0,
            .priceVolatility = 5.0,
            .minQuantity = 1,
            .maxQuantity = 100,
            .numMessages = 100,
            .seed = 1
        };

        MarketDataGenerator generator(config);
        auto messages = generator.generate();

        auto simStart = messages.front().timestamp;
        auto realStart = chrono::steady_clock::now();

        for (const auto& msg : messages) {
            if (!running_) break;

            switch (replayMode_) {
                case ReplayMode::REALTIME:
                    this_thread::sleep_until(realStart + (msg.timestamp - simStart));
                    break;
                case ReplayMode::ACCELERATED:
                    this_thread::sleep_until(realStart + getReplayOffset(simStart, msg.timestamp));
                    break;
                case ReplayMode::FIXED_DELAY:
                    this_thread::sleep_for(getReplayDelay());
                    break;
            }

            MarketDataMessage emitted = msg;
            emitted.timestamp = chrono::system_clock::now();
            generatedSink_(emitted);
        }
    }
}