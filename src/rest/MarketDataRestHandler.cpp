#include "../../include/rest/MarketDataRestHandler.h"

#include <crow.h>
#include <sstream>
#include <iomanip>

using namespace std;

MarketDataRestApi::MarketDataRestApi(std::shared_ptr<MarketDataStatsTracker> statsTracker): 
    statsTracker_(std::move(statsTracker)), 
    running_(false) 
    { }

MarketDataRestApi::~MarketDataRestApi() {
    stop();
}

void MarketDataRestApi::start(uint16_t port) {
    if (running_) return;

    running_ = true;
    serverThread_ = thread(&MarketDataRestApi::runServer, this, port);
}

void MarketDataRestApi::stop() {
    if (!running_) return;

    running_ = false;
    app_.stop();
    if (serverThread_.joinable()) serverThread_.join();
}

void MarketDataRestApi::runServer(uint16_t port) {
    // Define a route for stats per symbol
    CROW_ROUTE(app_, "/stats/<string>")
    ([this](const std::string& symbol){
        auto stats = statsTracker_->getStats(symbol);

        // JSON response with formatted stats
        crow::json::wvalue result;
        result["symbol"] = symbol;
        result["lastPrice"] = stats.lastPrice;
        result["totalVolume"] = stats.totalVolume;
        result["tradeCount"] = stats.tradeCount;
        result["highPrice"] = stats.highPrice;
        result["lowPrice"] = stats.lowPrice;
        result["averagePrice"] = stats.getAveragePrice();

        return result;
    });

    app_.port(port).multithreaded().run();

    running_ = false;
}