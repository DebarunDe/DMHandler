#pragma once

#include "../MarketDataSubscriber.h"

#include <iostream>
#include <iomanip>

class LoggingSubscriber : public IMarketDataSubscriber {
    public:
        void onMarketData(const MarketDataMessage& message) override {
            std::cout << "[LOG] "
                    << message.symbol << " "
                    << to_string(message.side) << ""
                    << " @"
                    << std::fixed << std::setprecision(2) << message.price << " "
                    << "x" << message.quantity 
            << std::endl;
        }
    };