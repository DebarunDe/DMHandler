#include "../include/MarketDataMessage.h"
#include "../include/MarketDataStatsTracker.h"
#include "../include/SymbolStats.h"

#include <mutex>
#include <iostream>


void MarketDataStatsTracker::update(const MarketDataMessage& message) {
    std::lock_guard<std::mutex> lock(statsMutex_);
    auto& stats = stats_[message.symbol];
    stats.update(message);
}

SymbolStats MarketDataStatsTracker::getStats(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(statsMutex_);

    auto it = stats_.find(symbol);
    if (it != stats_.end()) return it->second;
    return SymbolStats();
}

std::vector<std::string> MarketDataStatsTracker::getAllSymbols() const {
    std::lock_guard<std::mutex> lock(statsMutex_);

    std::vector<std::string> symbols;
    symbols.reserve(stats_.size());
    for (const auto& pair : stats_) {
        symbols.push_back(pair.first);
    }
    return symbols;
}