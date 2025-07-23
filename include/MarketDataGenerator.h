#pragma once

#include "MarketDataMessage.h"
#include "OrderSide.h"

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <random>

struct MarketDataGeneratorConfig {
    std::vector<std::string> symbols = {"AAPL", "GOOGL", "TSLA", "MSFT"}; // List of ticker symbols to generate data for
    double basePrice = 100.0; // Base price for generated data
    double priceVolatility = 2.0; // Price volatility factor
    double minQuantity = 10;
    double maxQuantity = 500;
    size_t numMessages = 100;
    std::optional<uint32_t> seed = std::nullopt;
};

class MarketDataGenerator {
private:
    MarketDataGeneratorConfig config_;
    std::mt19937 rng_; // Random number generator

    void validateConfig() const;

public:
    explicit MarketDataGenerator(const MarketDataGeneratorConfig& config);
    
    std::vector<MarketDataMessage> generate();
};