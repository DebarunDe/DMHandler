#pragma once

#include "../MarketDataMessage.h"
#include "../OrderSide.h"

#include <string>
#include <optional>

class MarketDataParser {
public:
    virtual std::optional<MarketDataMessage> parse(const std::string& line) = 0;
    virtual std::optional<MarketDataMessage> parse(const MarketDataMessage& line) = 0;
    virtual ~MarketDataParser() = default;
};