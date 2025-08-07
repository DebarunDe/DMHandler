#pragma once

#include "MarketDataMessage.h"

class IMarketDataSubscriber {
private:

public:
    virtual void onMarketData(const MarketDataMessage& message) = 0;
    virtual ~IMarketDataSubscriber() = default;
};