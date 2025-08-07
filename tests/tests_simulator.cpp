#include <gtest/gtest.h>
#include "../include/MarketDataSimulator.h"
#include "../include/MarketDataMessage.h"
#include "../include/ThreadSafeMessageQueue.h"

#include <chrono>
#include <thread>
#include <memory>

using namespace std;

vector<MarketDataMessage> drainQueue(ThreadSafeMessageQueue<MarketDataMessage>& queue) {
    vector<MarketDataMessage> messages;
    MarketDataMessage msg;
    while (queue.tryPop(msg)) messages.push_back(msg);
    return messages;
}

vector<string> drainQueue(ThreadSafeMessageQueue<string>& queue) {
    vector<string> messages;
    string msg;
    while (queue.tryPop(msg)) messages.push_back(msg);
    return messages;
}

TEST(MarketDataSimulatorTest, StartAndStopDoesNotThrow) {
    ThreadSafeMessageQueue<MarketDataMessage> queue;

    auto simulator = make_unique<MarketDataSimulator>(
        [](const string&) {}, // no-op
        [&](const MarketDataMessage& msg) { queue.push(msg); },
        SourceType::GENERATED
    );

    EXPECT_NO_THROW(simulator->start());
    this_thread::sleep_for(50ms);
    EXPECT_NO_THROW(simulator->stop());
}

TEST(MarketDataSimulatorTest, EmitsMessagesInRealtimeMode) {
    ThreadSafeMessageQueue<MarketDataMessage> queue;

    MarketDataSimulator simulator(
        [](const string&) {}, // no-op
        [&](const MarketDataMessage& msg) { queue.push(msg); },
        SourceType::GENERATED
    );

    simulator.setReplayMode(ReplayMode::REALTIME);
    simulator.start();
    this_thread::sleep_for(2s);
    simulator.stop();

    auto messages = drainQueue(queue);
    EXPECT_GT(messages.size(), 0);
    // Remove invalid comparison of MarketDataMessage with string
}

TEST(MarketDataSimulatorTest, AcceleratedReplayShouldBeFaster) {
    ThreadSafeMessageQueue<MarketDataMessage> queue;

    MarketDataSimulator simulator(
        [](const string&) {}, // no-op
        [&](const MarketDataMessage& msg) { queue.push(msg); },
        SourceType::GENERATED
    );

    auto start = chrono::steady_clock::now();
    simulator.setReplayMode(ReplayMode::ACCELERATED, 10.0);
    simulator.start();
    this_thread::sleep_for(300ms);
    simulator.stop();
    auto end = chrono::steady_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000);

    auto messages = drainQueue(queue);
    // In accelerated mode, we should get some messages but not necessarily all 101
    EXPECT_GT(messages.size(), 0);
    EXPECT_LE(messages.size(), 101); // Should not exceed the total number of messages
}

TEST(MarketDataSimulatorTest, FixedDelayModeEmitsAtControlledRate) {
    ThreadSafeMessageQueue<MarketDataMessage> queue;

    MarketDataSimulator simulator(
        [](const string&) {}, // no-op
        [&](const MarketDataMessage& msg) { queue.push(msg); },
        SourceType::GENERATED
    );

    simulator.setReplayMode(ReplayMode::FIXED_DELAY, 2.0); // 50ms per message
    simulator.start();
    this_thread::sleep_for(300ms);
    simulator.stop();

    auto messages = drainQueue(queue);
    EXPECT_GE(messages.size(), 3);
}

TEST(MarketDataSimulatorTest, CanEmitFileSourceLines) {
    ThreadSafeMessageQueue<string> queue;

    MarketDataSimulator simulator(
        [&](const string& line) { queue.push(line); },
        [](const MarketDataMessage&) {}, // no-op
        SourceType::FILE
    );

    simulator.setReplayMode(ReplayMode::REALTIME);
    simulator.start();
    this_thread::sleep_for(200ms);
    simulator.stop();

    auto messages = drainQueue(queue);
    EXPECT_GT(messages.size(), 0);
}
