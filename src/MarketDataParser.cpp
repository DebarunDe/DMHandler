#include "../include/MarketDataParser.h"
#include "../include/MarketDataMessage.h"
#include "../include/OrderSide.h"

#include <string>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <optional>

using namespace std;

inline std::string trim(const std::string& s) {
    const auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return ""; // string is all whitespace
    const auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

optional<MarketDataMessage> MarketDataParser::parse(const std::string& line) {
    istringstream iss(line);

    string symbol, sideStr, priceStr, sizeStr, timestampStr;

    if (!getline(iss, symbol, ','))     return nullopt;
    if (!getline(iss, sideStr, ','))    return nullopt;
    if (!getline(iss, priceStr, ','))   return nullopt;
    if (!getline(iss, sizeStr, ','))    return nullopt;
    if (!getline(iss, timestampStr))    return nullopt;

    // trim whitespace
    symbol = trim(symbol);
    sideStr = trim(sideStr);
    priceStr = trim(priceStr);
    sizeStr = trim(sizeStr);
    timestampStr = trim(timestampStr);

    try {
        double price = stod(priceStr);
        int quantity = stoi(sizeStr);
        OrderSide side = from_string(sideStr);
        auto timestamp = chrono::system_clock::time_point(chrono::duration_cast<chrono::system_clock::duration>(chrono::nanoseconds(stoll(timestampStr))));

        return MarketDataMessage{ symbol, side, price, quantity, timestamp };
    } catch (...) {
        return nullopt; // Return nullopt if any parsing error occurs
    }
}
