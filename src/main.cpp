#include "../include/MarketDataSimulator.h"
#include <iostream>

using namespace std;

int main() {
    //clear && cmake && make && ./market_data_feed
    // try {
    //     MarketDataSimulator simulator("../data/market_data.csv",true); //Static case

    //     simulator.run([](const string& line) {
    //         // Callback function to process each line of market data
    //         cout << "Received market data: " << line << endl;
    //     });
    // }
    // catch (const exception& ex) {
    //     cerr << "Error: " << ex.what() << endl;
    // }

    try {
        MarketDataSimulator simulator(10000,false,1000000); //Static case

        simulator.run([](const string& line) {
            // Callback function to process each line of market data
            cout << "Received market data: " << line << endl;
        });
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
    }

    return 0;
}