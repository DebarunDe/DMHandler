#include "../include/MarketDataSimulator.h"
#include <iostream>

using namespace std;

int main() {

    try {
        MarketDataSimulator simulator("../data/market_data.csv",true); //Static case

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