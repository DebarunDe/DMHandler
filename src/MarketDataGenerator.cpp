#include "../include/MarketDataGenerator.h"

#include <stdexcept>

using namespace std;

void MarketDataGenerator::validateConfig() const {
    if (config_.symbols.empty())                                throw invalid_argument("Symbol list cannot be empty");
    if (config_.numMessages == 0)                               throw invalid_argument("Number of messages must be greater than zero");
    if (config_.basePrice <= 0)                                 throw invalid_argument("Base price must be greater than zero");
    if (config_.minQuantity <= 0 || config_.maxQuantity <= 0)   throw invalid_argument("Quantities must be greater than zero");
    if (config_.minQuantity > config_.maxQuantity)              throw invalid_argument("Minimum quanttity cannot be greater than maximum quantity");
    if (config_.priceVolatility < 0)                            throw invalid_argument("Price volatility cannot be negative");
}

MarketDataGenerator::MarketDataGenerator(const MarketDataGeneratorConfig& config):
    config_ ( config ),
    rng_    ( config.seed.has_value()? std::mt19937(*config.seed) : std::mt19937(random_device{}()) )
    { 
        validateConfig();
    }

vector<MarketDataMessage> MarketDataGenerator::generate() {
    vector<MarketDataMessage> messages;
    messages.reserve(config_.numMessages);

    uniform_real_distribution<double> priceDist(
        config_.basePrice - config_.priceVolatility,
        config_.basePrice + config_.priceVolatility
    );
    uniform_real_distribution<double> quantityDist(
        config_.minQuantity,
        config_.maxQuantity
    );
    uniform_int_distribution<int> sideDist(0,1); // 0 for BUY, 1 for SELL
    uniform_int_distribution<size_t> symbolIndexDist(0, config_.symbols.size() - 1);

    auto now = chrono::steady_clock::now();

    for (size_t i = 0; i < config_.numMessages; ++i) {
        MarketDataMessage msg;

        msg.symbol = config_.symbols[symbolIndexDist(rng_)];
        msg.price = priceDist(rng_);
        msg.quantity = static_cast<int>(quantityDist(rng_));
        msg.side = static_cast<OrderSide>(sideDist(rng_));
        msg.timestamp = now + chrono::milliseconds(i * 50); // Increment timestamp by 50ms for each message

        messages.push_back(msg);
    }

    return messages;
}