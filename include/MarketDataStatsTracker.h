#pragma once

#include "MarketDataMessage.h"
#include "SymbolStats.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class MarketDataStatsTracker {
    private:
        mutable std::mutex statsMutex_;
        std::unordered_map<std::string, SymbolStats> stats_;
    
    public:
        void update(const MarketDataMessage& message);
    
        SymbolStats getStats(const std::string& symbol) const;
        std::vector<std::string> getAllSymbols() const;
    };