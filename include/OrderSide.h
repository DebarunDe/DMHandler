#pragma once

#include <string>
#include <stdexcept>

enum class OrderSide {
    BUY,
    SELL,
    UNKNOWN
};

inline std::string to_string(OrderSide side) {
    switch (side) {
        case OrderSide::BUY:  return "BUY";
        case OrderSide::SELL: return "SELL";
        default:              return "UNKNOWN";
    }
}

inline OrderSide from_string(const std::string& str) {
    if (str == "BUY")  return OrderSide::BUY;
    if (str == "SELL") return OrderSide::SELL;
    if (str == "UNKNOWN") return OrderSide::UNKNOWN;
    throw std::invalid_argument("Invalid OrderSide string: " + str);
}