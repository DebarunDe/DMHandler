#pragma once

#include "OrderSide.h"
#include "MarketDataMessage.h"

#include <string>
#include <optional>

class MarketDataParser {
private:

public:
    // Parse a market data message from a string
    static std::optional<MarketDataMessage> parse(const std::string& message); //string specialization
};