#pragma once

#include "../MarketDataSubscriber.h"

#include <iostream>

class LoggingSubscriber : public IMarketDataSubscriber {
    public:
        void onMarketData(const MarketDataMessage& message) override {
            std::cout << "Received Market Data: "
                    << "[LOG] "
                    << message.symbol << " "
                    << to_string(message.side) << ""
                    << " @"
                    << message.price << " "
                    << "x" << message.quantity 
            << std::endl;
        }
    };