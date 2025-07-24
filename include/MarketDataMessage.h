#pragma once

#include "OrderSide.h"
#include <string>
#include <chrono>

struct MarketDataMessage {
    std::string symbol;
    OrderSide side;
    double price;
    int quantity;
    std::chrono::system_clock::time_point timestamp;
};