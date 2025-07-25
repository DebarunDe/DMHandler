#pragma once

#include "OrderSide.h"
#include "MarketDataMessage.h"

#include <string>
#include <optional>

class MarketDataParser {
private:

public:
    MarketDataParser() = default;
    virtual ~MarketDataParser() = default;

    // Parse a market data message from a input
    static std::optional<MarketDataMessage> parse(const std::string& line); //string specialization
};