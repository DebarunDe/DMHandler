#include "../include/MarketDataSimulator.h"
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>

using namespace std;

/*
*   enums to create dynamic data
*/
using TickerSymbol = string;
using OrderType = string;
using Price = double;
using Quantity = int;
using Timestamp = long long;

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

enum Order {
    BUY     = 0,
    SELL    = 1
};
OrderType ordertypeToString(Order order) {
    switch (order) {
        case BUY: return "BUY";
        case SELL: return "SELL";
        default: return "UNKNOWN";
    }
}


MarketDataSimulator::MarketDataSimulator():
        dataSource_("/Users/debarunde/VSCode/DMHandler/DMHandler/data/market_data.csv"),
        isStatic_(false),
        runUniform_(true),
        msgPerSecond_(1'000), // default to 1000 messages per second
        msgCount_(1'000)
        { }

//primarily used for static case from csv file
MarketDataSimulator::MarketDataSimulator(bool isStatic):
    dataSource_("/Users/debarunde/VSCode/DMHandler/DMHandler/data/market_data.csv"),
    isStatic_(isStatic),
    runUniform_(true),
    msgPerSecond_(1'000), // default to 1000 messages per second
    msgCount_(1'000)
    { }
    
//primary used for dynamic case
MarketDataSimulator::MarketDataSimulator(bool runUniform, int msgPerSecond, int msgCount):
    dataSource_(""),
    isStatic_(false),
    runUniform_(runUniform),
    msgPerSecond_(msgPerSecond),
    msgCount_(msgCount)
    { }

MarketDataSimulator::~MarketDataSimulator() {
    // destructor logic if needed
}


void MarketDataSimulator::sleep(chrono::microseconds interval) {
    if (isRunUniform()) this_thread::sleep_for(interval);
    else {
        auto devianceType = arc4random() % 2; // 0 for burst, 1 for slowdown
        (devianceType == 0)?
            this_thread::sleep_for(interval / (1 - arc4random() % 10)): 
            this_thread::sleep_for(interval * (1 + arc4random() % 10)); //randomly increase/decrease the interval by 0-3 times
    }
}

void MarketDataSimulator::run(function<void(const string&)> callback) {
    auto interval = chrono::microseconds(1'000'000 / getMsgPerSecond()); // convert messages per second to milliseconds

    if (isStatic_) {
        //read from file
        ifstream file(dataSource_);
        
        if (!file.is_open()) throw runtime_error("Could not open file: " + dataSource_);

        string line;

        while (getline(file, line)) {
            callback(line);
            sleep(interval);
        }
    }
    else {
        //dynamically generate data
        auto dynamicMsgCount = 0;
        while (dynamicMsgCount < msgCount_) {
            //Generate single line of data
            TickerSymbol ticker = tickerToString(static_cast<Ticker>(arc4random() % 11)); // 0-10 for 11 symbols
            OrderType order = ordertypeToString(static_cast<Order>(arc4random() % 2)); // 0 or 1 for buy/sell
            Price price = 100.0 + static_cast<double>(arc4random() % 10'000) / 100.0; // price between 100.00 and 200.00
            ostringstream priceStream;
            priceStream << fixed << setprecision(2) << price; // format price to 2 decimal places
            Quantity quantity = 1 + arc4random() % 100; // quantity between 1 and 100
            Timestamp timestamp = chrono::duration_cast<chrono::microseconds>(
                chrono::system_clock::now().time_since_epoch()
            ).count();
            
            string dataLine = ticker + "," +
                                priceStream.str() + "," + 
                                to_string(quantity) + "," +
                                order + "," +
                                to_string(timestamp);
            callback(dataLine);
            dynamicMsgCount++;
            sleep(interval);
        }
    }
}