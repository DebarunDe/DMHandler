#pragma once

#include "OrderSide.h"
#include <string>
#include <chrono>

struct MarketDataMessage {
    std::string symbol;
    double price;
    int quantity;
    OrderSide side;
    std::chrono::steady_clock::time_point timestamp;
};