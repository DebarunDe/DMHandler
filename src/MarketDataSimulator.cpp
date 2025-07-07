#include "../include/MarketDataSimulator.h"
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

MarketDataSimulator::MarketDataSimulator():
        dataSource_("/Users/debarunde/VSCode/DMHandler/DMHandler/data/market_data.csv"),
        isStatic_(false),
        msgPerSecond_(1000), // default to 1000 messages per second
        runUniform_(true)
        { }

//primarily used for static case from csv file
MarketDataSimulator::MarketDataSimulator(const string& dataSource, bool isStatic):
    dataSource_(dataSource),
    isStatic_(isStatic),
    msgPerSecond_(1000), // default to 1000 messages per second
    runUniform_(true)
    { }
    
//primary used for dynamic case
// MarketDataSimulator::MarketDataSimulator(bool isStatic = false, int msgPerSecond, bool runUniform, int msgCount):
//     dataSource_(""),
//     isStatic_(isStatic),
//     msgPerSecond_(msgPerSecond),
//     runUniform_(runUniform),
//     msgCount_(msgCount)
//     { }

MarketDataSimulator::~MarketDataSimulator() {
    // destructor logic if needed
}

void MarketDataSimulator::run(function<void(const string&)> callback) {
    auto interval = chrono::microseconds(1'000'000 / msgPerSecond_); // convert messages per second to milliseconds
    
    if (isStatic_) {
        //read from file
        ifstream file(dataSource_);
        
        if (!file.is_open()) throw runtime_error("Could not open file: " + dataSource_);

        string line;

        while (getline(file, line)) {
            callback(line);
            if (runUniform_) this_thread::sleep_for(interval);
            else {
                //simulate bursts/slowdowns
                this_thread::sleep_for(interval * (1 +rand() % 3)); //randomly increase the interval by 0-3 times
            }
        }
    }
    else {
        //dynamically generate data
    }
}