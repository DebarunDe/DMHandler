#include <gtest/gtest.h>
#include <cstdlib> // for getenv
#include "../include/MarketDataSimulator.h"
#include "../include/MarketDataMessage.h"
#include "../include/ThreadSafeMessageQueue.h"

#include <chrono>
#include <thread>

using namespace std;
//clear && cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build && ./build/tests_simulator

class MarketDataSimulatorTest : public ::testing::Test {
protected:
    ThreadSafeMessageQueue<MarketDataMessage> queue;
    unique_ptr<MarketDataSimulator> simulator;
    
    void SetUp()    override { simulator = make_unique<MarketDataSimulator>(queue); }
    void TearDown() override { simulator->stop(); } //always stop the simulator 
    
    vector<MarketDataMessage> drainQueue() {
        vector<MarketDataMessage> messages;
        MarketDataMessage msg;
        while (queue.tryPop(msg)) messages.push_back(msg);
        return messages;
    }
    
};

TEST_F(MarketDataSimulatorTest, StartAndStopDoesNotThrow) {
    EXPECT_NO_THROW(simulator->start());
    this_thread::sleep_for(50ms); // let it do a little work
    EXPECT_NO_THROW(simulator->stop());
}

TEST_F(MarketDataSimulatorTest, EmitsMessagesInRealtimeMode) {
    simulator->setReplayMode(ReplayMode::REALTIME);
    simulator->start();
    this_thread::sleep_for(15s); // Let it emit all messages
    simulator->stop();

    auto messages = drainQueue();
    EXPECT_GT(messages.size(), 0);
    EXPECT_EQ(messages.front().symbol, "TSLA");
    EXPECT_EQ(messages.back().symbol, "JPM");

    const auto& frontMessage = messages.front();
    EXPECT_FALSE(frontMessage.symbol.empty());
    EXPECT_GE(frontMessage.price, 0.0); // Price can be zero or positive
    EXPECT_GE(frontMessage.quantity, 0); // Volume can be zero or positive
    EXPECT_TRUE(frontMessage.side == OrderSide::BUY || frontMessage.side == OrderSide::SELL);
    EXPECT_GE(frontMessage.timestamp.time_since_epoch().count(), 0); // Timestamp should be non-negative
}

TEST_F(MarketDataSimulatorTest, AcceleratedReplayShouldBeFaster) {
    auto start = chrono::steady_clock::now();
    simulator->setReplayMode(ReplayMode::ACCELERATED, 10.0); // 10x faster
    simulator->start();
    this_thread::sleep_for(300ms);
    simulator->stop();
    auto end = chrono::steady_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000); // Should finish under 1s

    auto messages = drainQueue();
    EXPECT_EQ(messages.size(), 101); // All 101 should've been emitted
}

TEST_F(MarketDataSimulatorTest, FixedDelayModeEmitsAtControlledRate) {
    simulator->setReplayMode(ReplayMode::FIXED_DELAY, 2.0); // 50ms per message
    simulator->start();
    this_thread::sleep_for(300ms);
    simulator->stop();

    auto messages = drainQueue();
    EXPECT_GE(messages.size(), 3); // Should emit ~3 messages in 300ms
}

TEST_F(MarketDataSimulatorTest, CanSwitchToFileSource) {
    simulator->setSourceType(SourceType::FILE);
    simulator->setReplayMode(ReplayMode::REALTIME);
    
    EXPECT_NO_THROW(simulator->start());
    this_thread::sleep_for(50ms); // Let it do a little work
    EXPECT_NO_THROW(simulator->stop());

    auto messages = drainQueue();
    EXPECT_GT(messages.size(), 0);
}

TEST_F(MarketDataSimulatorTest, CanSwitchToGeneratedSource) {
    simulator->setSourceType(SourceType::GENERATED);
    simulator->setReplayMode(ReplayMode::REALTIME);
    
    EXPECT_NO_THROW(simulator->start());
    this_thread::sleep_for(50ms); // Let it do a little work
    EXPECT_NO_THROW(simulator->stop());

    // Check if messages were generated
    EXPECT_GE(queue.size(), 1); // At least one message should be generated

    auto messages = drainQueue();
    EXPECT_GT(messages.size(), 0);

    const auto& frontMessage = messages.front();
    EXPECT_FALSE(frontMessage.symbol.empty());
    EXPECT_GE(frontMessage.price, 0.0); // Price can be zero or positive
    EXPECT_GE(frontMessage.quantity, 0); // Volume can be zero or positive
    EXPECT_TRUE(frontMessage.side == OrderSide::BUY || frontMessage.side == OrderSide::SELL);
    EXPECT_GE(frontMessage.timestamp.time_since_epoch().count(), 0); // Timestamp should be non-negative
}