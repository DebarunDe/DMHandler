#pragma once

#include "../OrderSide.h"
#include "../MarketDataMessage.h"
#include "MarketDataParser.h"

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>

using CreatorFunction = std::function<std::unique_ptr<MarketDataParser>()>;
class MarketDataParserFactory {
private:
    static std::unordered_map<std::string, CreatorFunction>& getRegistry();

public:
    static void registerParser(const std::string& type, CreatorFunction creator);
    static std::unique_ptr<MarketDataParser> create(const std::string& name);
};