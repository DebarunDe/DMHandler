#pragma once

#include "MarketDataParser.h"

#include "nlohmann/json.hpp"

class FinnhubMarketDataParser : public MarketDataParser {
public:
    std::optional<MarketDataMessage> parse(const std::string& line) override;
    std::optional<MarketDataMessage> parse(const MarketDataMessage& line) override;
};