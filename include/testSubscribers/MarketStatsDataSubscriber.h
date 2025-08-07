#pragma once

#include "../MarketDataSubscriber.h"
#include "../MarketDataStatsTracker.h"
#include "../SymbolStats.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <memory>


class MarketDataStatsSubscriber : public IMarketDataSubscriber {
private:
    std::shared_ptr<MarketDataStatsTracker> statsTracker_;
public:
    explicit MarketDataStatsSubscriber(std::shared_ptr<MarketDataStatsTracker> tracker): 
    statsTracker_(std::move(tracker)) 
    { }

    void onMarketData(const MarketDataMessage& message) override {
        return;
    }

    SymbolStats getStats(const std::string& symbol) const {
        return statsTracker_->getStats(symbol);
    }
};