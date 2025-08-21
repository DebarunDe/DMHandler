#include "../../include/parser/FinnhubMarketDataParser.h"


#include <chrono>

using namespace std;
using json = nlohmann::json;

optional<MarketDataMessage> FinnhubMarketDataParser::parse(const std::string& line) {
    try {
        auto j = json::parse(line);

        if (j.contains("type") && j["type"] == "trade" && j.contains("data") && j["data"].is_array()) {
            auto item = j["data"];
            if (!item.empty() && item[0].is_object()) {
                const auto &t = item[0];
                MarketDataMessage msg;

                msg.symbol = t.value("s", string());
                msg.price = t.value("p", 0.0);
                msg.quantity = t.value("v", 0);
                msg.side = OrderSide::UNKNOWN; // Finnhub does not provide side info

                long long epoch = t.value("t", 0ll);
                if (epoch > 1000000000000ll) { // > ~2001-09-09 in ms
                    std::chrono::system_clock::time_point tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(epoch));
                    msg.timestamp = tp;
                } else {
                    std::chrono::system_clock::time_point tp = std::chrono::system_clock::time_point(std::chrono::seconds(epoch));
                    msg.timestamp = tp;
                }

                return msg;
            }
        }
    } catch (...) { }

    return nullopt;
}

optional<MarketDataMessage> FinnhubMarketDataParser::parse(const MarketDataMessage& line) {
    // Pass-through or basic validation logic
    return line;
}