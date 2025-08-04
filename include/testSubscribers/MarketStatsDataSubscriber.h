#pragma once

#include "../MarketDataSubscriber.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <memory>
#include <unordered_map>

struct SymbolStats {
    double lastPrice = 0.00;
    uint64_t totalVolume = 0;
    uint64_t tradeCount = 0;
    double highPrice = std::numeric_limits<double>::lowest();
    double lowPrice = std::numeric_limits<double>::max();
    double totalNotional = 0.00;

    void update(const MarketDataMessage& message) {
        lastPrice = message.price;
        totalVolume += message.quantity;
        tradeCount++;
        highPrice = std::max(highPrice, message.price);
        lowPrice = std::min(lowPrice, message.price);
        totalNotional += message.price * message.quantity;
    }

    double getAveragePrice() const {
        return totalVolume > 0? totalNotional / totalVolume : 0.00;
    }
};

class MarketDataStatsTracker {
private:
    mutable std::mutex statsMutex_;
    std::unordered_map<std::string, SymbolStats> stats_;

public:
    void update(const MarketDataMessage& message) {
        std::lock_guard<std::mutex> lock(statsMutex_);

        auto& stats = stats_[message.symbol];
        stats.update(message);
    }

    SymbolStats getStats(const std::string& symbol) const {
        std::lock_guard<std::mutex> lock(statsMutex_);

        auto it = stats_.find(symbol);
        if (it != stats_.end()) return it->second;
        return SymbolStats();
    }
};

class MarketDataStatsSubscriber : public IMarketDataSubscriber {
private:
    std::shared_ptr<MarketDataStatsTracker> statsTracker_;
public:
    explicit MarketDataStatsSubscriber(std::shared_ptr<MarketDataStatsTracker> tracker): 
    statsTracker_(std::move(tracker)) 
    { }

    void onMarketData(const MarketDataMessage& message) override {
        statsTracker_->update(message);
    }

    SymbolStats getStats(const std::string& symbol) const {
        return statsTracker_->getStats(symbol);
    }
};