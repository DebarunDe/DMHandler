#pragma once

#include "MarketDataParser.h"
#include "../MarketDataMessage.h"

class GeneratedMarketDataParser : public MarketDataParser {
public:
    std::optional<MarketDataMessage> parse(const std::string& line) override;
    std::optional<MarketDataMessage> parse(const MarketDataMessage& message) override;
};