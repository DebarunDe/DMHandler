#include <gtest/gtest.h>
#include "../include/MarketDataSimulator.h"
#include <cstdlib> // for getenv

using namespace std;
//clear && cmake .. && make && ctest 

TEST(MarketDataSimulatorTest, DefaultConstructor) {
    MarketDataSimulator sim;
    EXPECT_EQ(sim.getDataSource(), "/Users/debarunde/VSCode/DMHandler/DMHandler/data/market_data.csv");
    EXPECT_FALSE(sim.isStatic());
    EXPECT_EQ(sim.getMsgPerSecond(), 1'000);
    EXPECT_TRUE(sim.isRunUniform());
    EXPECT_EQ(sim.getMsgCount(),1'000);
}

TEST(MarketDataSimulatorTest, StaticConstructor) {
    MarketDataSimulator sim(true);
    EXPECT_EQ(sim.getDataSource(), "/Users/debarunde/VSCode/DMHandler/DMHandler/data/market_data.csv");
    EXPECT_TRUE(sim.isStatic());
    EXPECT_EQ(sim.getMsgPerSecond(), 1'000);
    EXPECT_TRUE(sim.isRunUniform());
    EXPECT_EQ(sim.getMsgCount(), 1'000);
}

TEST(MarketDataSimulatorTest, DynamicConstructor) {
    MarketDataSimulator sim(false, 500, 2'000);
    EXPECT_EQ(sim.getDataSource(), "");
    EXPECT_FALSE(sim.isStatic());
    EXPECT_EQ(sim.getMsgPerSecond(), 500);
    EXPECT_FALSE(sim.isRunUniform());
    EXPECT_EQ(sim.getMsgCount(), 2'000);
}

TEST(MarketDataSimulatorTest, StaticReadsLinesCorrectly) {
    int count = 0;
    MarketDataSimulator sim; // Fast for test

    sim.run([&count](const string& line) {
        EXPECT_FALSE(line.empty());
        count++;
    });

    EXPECT_GT(count, 0); // Expect at least one line was read
}

TEST(MarketDataSimulatorTest, DynamicGeneratesLinesCorrectlyUniform) {
    int count = 0;
    auto msgPerSecond = 1'000;
    auto runUniform = true;
    auto msgCount = 10'000;
    MarketDataSimulator sim(runUniform, msgPerSecond, msgCount); // Fast for test

    sim.run([&count](const string& line){
        EXPECT_FALSE(line.empty());
        count++;

        // validate format of line
        vector<string> parts;
        stringstream ss(line);
        string part;
        while (getline(ss, part, ',')) parts.push_back(part); 
        EXPECT_EQ(parts.size(), 5); // Expect 5 parts: ticker, price, quantity, order type, timestamp
        EXPECT_FALSE(parts[0].empty()); // Ticker should not be empty
        EXPECT_TRUE(stod(parts[1]) > 0); // Price should be a positive number
        EXPECT_TRUE(stoi(parts[2]) > 0); // Quantity should be a positive integer
        EXPECT_TRUE(parts[3] == "BUY" || parts[3] == "SELL"); // Order type should be either buy or sell
        EXPECT_TRUE(stoll(parts[4]) > 0); // Timestamp should be a positive long
    });

    EXPECT_EQ(count, 10'000); // Expect exactly 10000 lines generated 
}

TEST(MarketDataSimulatorTest, DynamicGeneratesLinesCorrectlyNonUniform) {
    if (std::getenv("CI") != nullptr) GTEST_SKIP() << "Skipping test in CI environment due to flakiness";
    
    int count = 0;
    auto msgPerSecond = 1'000;
    auto runUniform = false;
    auto msgCount = 10'000;
    MarketDataSimulator sim(msgPerSecond, runUniform, msgCount); // Fast for test

    sim.run([&count](const string& line){
        EXPECT_FALSE(line.empty());
        count++;

        // validate format of line
        vector<string> parts;
        stringstream ss(line);
        string part;
        while (getline(ss, part, ',')) parts.push_back(part); 
        EXPECT_EQ(parts.size(), 5); // Expect 5 parts: ticker, price, quantity, order type, timestamp
        EXPECT_FALSE(parts[0].empty()); // Ticker should not be empty
        EXPECT_TRUE(stod(parts[1]) > 0); // Price should be a positive number
        EXPECT_TRUE(stoi(parts[2]) > 0); // Quantity should be a positive integer
        EXPECT_TRUE(parts[3] == "BUY" || parts[3] == "SELL"); // Order type should be either buy or sell
        EXPECT_TRUE(stoll(parts[4]) > 0); // Timestamp should be a positive long
    });

    EXPECT_EQ(count, 10'000); // Expect exactly 10000 lines generated 
}