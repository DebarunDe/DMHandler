#pragma once

#include <string>

using TickerSymbol = std::string;

enum Ticker {
    AAPL    = 0,
    TSLA    = 1,
    GOOGL   = 2,
    AMZN    = 3,
    MSFT    = 4,
    FB      = 5,
    NFLX    = 6,
    NVDA    = 7,
    AMD     = 8,
    BABA    = 9,
    DIS     = 10
};
TickerSymbol tickerToString(Ticker ticker) {
    switch (ticker) {
        case AAPL: return "AAPL";
        case TSLA: return "TSLA";
        case GOOGL: return "GOOGL";
        case AMZN: return "AMZN";
        case MSFT: return "MSFT";
        case FB: return "FB";
        case NFLX: return "NFLX";
        case NVDA: return "NVDA";
        case AMD: return "AMD";
        case BABA: return "BABA";
        case DIS: return "DIS";
        default: return "UNKNOWN";
    }
}