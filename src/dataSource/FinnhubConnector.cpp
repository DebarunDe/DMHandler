#include "../../include/dataSource/FinnhubConnector.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <optional>
#include <algorithm>

using namespace std;
using json = nlohmann::json;

FinnhubConnector::FinnhubConnector(
    unique_ptr<IxWebSocketClient> wsClient,
    unique_ptr<FinnhubMarketDataParser> parser,
    ThreadSafeMessageQueue<MarketDataMessage>& messageQueue,
    vector<string> symbols
):
    wsClient_(std::move(wsClient)),
    parser_(std::move(parser)),
    messageQueue_(messageQueue),
    subscribedSymbols_(std::move(symbols)),
    running_(false)
    {
        wsClient_->setMessageCallBack(
            [this](const string& message) {
                onMessageReceived(message);
        });
    }

FinnhubConnector::~FinnhubConnector() { stop(); }

void FinnhubConnector::start() {
    if (running_) return;

    running_ = true;
    teardownRequested_ = false;
    workerThread_ = thread(&FinnhubConnector::ingestLoop, this);
}

void FinnhubConnector::stop() {
    if (!running_) return;

    running_ = false;
    teardownRequested_ = true;

    try {
        if (wsClient_ && wsClient_->isConnected()) wsClient_->disconnect();
    } catch(...) {
        cerr << "[ERROR] Failed to disconnect WebSocket client.\n";
    }

    if (workerThread_.joinable()) workerThread_.join();
    cout << "[INFO] Disconnected from Finnhub WebSocket.\n";
}

void FinnhubConnector::subscribeToSymbol(const string& symbol) {
    if (symbol.empty()) {
        cerr << "[ERROR] Cannot subscribe to an empty symbol.\n";
        return;
    }

    {
        std::lock_guard<std::mutex> lock(subscribedSymbolsMutex_);
        if (find(subscribedSymbols_.begin(), subscribedSymbols_.end(), symbol) != subscribedSymbols_.end()) {
            cout << "[INFO] Already subscribed to symbol: " << symbol << "\n";
            return;
        }
    }

    if (wsClient_ && wsClient_->isConnected()) sendSubscribe(symbol);
    else cerr << "[ERROR] Cannot subscribe to symbol: " << symbol << " - WebSocket is not connected.\n";
}

bool FinnhubConnector::isRunning() const { 
    return running_; 
}

void FinnhubConnector::tryConnect() {
    if (!wsClient_) {
        cerr << "[ERROR] WebSocket client is not initialized.\n";
        return;
    }
    if (wsClient_->isConnected()) return;

    try {
        cout << "[INFO] Attempting to connect to Finnhub WebSocket...\n";
        wsClient_->connect();
        vector<string> symbols;

        {
            std::lock_guard<std::mutex> lock(subscribedSymbolsMutex_);
            symbols = subscribedSymbols_;
        }
        for (const auto& symbol : symbols) sendSubscribe(symbol);

        cout << "[INFO] Successfully connected to Finnhub WebSocket.\n";
    } catch (const exception& e) {
        cerr << "[ERROR] Failed to connect to Finnhub WebSocket: " << e.what() << "\n";
    }
}

void FinnhubConnector::sendSubscribe(const string& symbol) {
    if (!wsClient_ || !wsClient_->isConnected()) {
        cerr << "[ERROR] Cannot subscribe to symbol " << symbol << ": WebSocket is not connected.\n";
        return;
    }

    json subscribeMessage;
    subscribeMessage["type"] = "subscribe";
    subscribeMessage["symbol"] = symbol;

    {
        std::lock_guard<std::mutex> lock(subscribedSymbolsMutex_);
        wsClient_->send(subscribeMessage.dump());
        if (std::find(subscribedSymbols_.begin(), subscribedSymbols_.end(), symbol) == subscribedSymbols_.end()) {
            cout << "[INFO] Subscribed to symbol: " << symbol << "\n";
            subscribedSymbols_.emplace_back(symbol);
        }
    }
}

void FinnhubConnector::onMessageReceived(const string& message) {
    if (!parser_) {
        cerr << "[ERROR] Market data parser is not initialized.\n";
        return;
    }

    try {
        auto parsedMessage = parser_->parse(message);
        if (parsedMessage.has_value()) messageQueue_.push(parsedMessage.value());
    } catch (const exception& e) {
        cerr << "[ERROR] Failed to parse message: " << e.what() << "raw: " << message << "\n";
    }
}

void FinnhubConnector::ingestLoop() {
    const chrono::milliseconds baseWait = 500ms;
    chrono::milliseconds wait = baseWait;

    while (running_) {
        tryConnect();

        if (wsClient_ && wsClient_->isConnected()) {
            wait = baseWait;

            this_thread::sleep_for(100ms);
            continue;
        }

        if (teardownRequested_) {
            cout << "[INFO] Teardown requested, stopping ingestion loop.\n";
            break;
        }

        cout << "[INFO] WebSocket not connected, retrying in " << wait.count() << " ms...\n";
        this_thread::sleep_for(wait);
        wait = min(wait * 2, chrono::milliseconds(10000)); // Exponential backoff, max 10 seconds
    }
}