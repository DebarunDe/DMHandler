#include "../include/MarketDataMessage.h"
#include "../include/MarketDataStatsTracker.h"
#include "../include/SymbolStats.h"

#include <mutex>


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