#include "../include/MarketDataSimulator.h"
#include "../include/ThreadSafeMessageQueue.h"
#include <iostream>

using namespace std;

int main() {
    // cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja -C build
    // cd build && ctest --output-on-failure && ./DMHandler
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

    // try {
    //     MarketDataSimulator simulator(10'000,false,1'000'000); //Static case

    //     simulator.run([](const string& line) {
    //         // Callback function to process each line of market data
    //         cout << "Received market data: " << line << endl;
    //     });
    // }
    // catch (const exception& ex) {
    //     cerr << "Error: " << ex.what() << endl;
    // }

    return 0;
}