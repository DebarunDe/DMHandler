#pragma once

#include "MarketDataMessage.h"

#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <memory>
#include <chrono>

struct SymbolStats {
    double lastPrice = 0.00;
    uint64_t totalVolume = 0;
    uint64_t tradeCount = 0;
    double highPrice = std::numeric_limits<double>::lowest();
    double lowPrice = std::numeric_limits<double>::max();
    double totalNotional = 0.00;
    std::chrono::system_clock::time_point lastUpdateTime = std::chrono::system_clock::now();

    void update(const MarketDataMessage& message) {
        lastPrice = message.price;
        totalVolume += message.quantity;
        tradeCount++;
        highPrice = std::max(highPrice, message.price);
        lowPrice = std::min(lowPrice, message.price);
        totalNotional += message.price * message.quantity;
        lastUpdateTime = message.timestamp;
    }

    double getAveragePrice() const {
        return totalVolume > 0? totalNotional / totalVolume : 0.00;
    }
};