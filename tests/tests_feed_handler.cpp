#include <gtest/gtest.h>
#include "../include/MarketDataFeedHandler.h"

#include "../include/testSubscribers/LoggingSubscriber.h"

using namespace std;

class MarketDataFeedHandlerTest : public ::testing::Test {
protected:
    ThreadSafeMessageQueue<MarketDataMessage> queue;
    unique_ptr<MarketDataFeedHandler> feedHandler;

    void SetUp()    override { 
        feedHandler = make_unique<MarketDataFeedHandler>(queue); 
        feedHandler->start(); 
    }
    void TearDown() override { 
        // always stop the feed handler
        feedHandler->stop(); 
    } 
};

TEST_F(MarketDataFeedHandlerTest, SubscribeAndUnsubscribe) {
    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    EXPECT_NO_THROW(feedHandler->subscribe(loggingSubscriber));
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber)); // Unsubscribing again should not throw
    EXPECT_NO_THROW(feedHandler->subscribe(loggingSubscriber)); // Re-subscribing should work
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
}

TEST_F(MarketDataFeedHandlerTest, nullSubscribeAndUnsubscribe) {
    auto nullSubscriber = shared_ptr<IMarketDataSubscriber>(nullptr);
    EXPECT_NO_THROW(feedHandler->subscribe(nullSubscriber));
    EXPECT_NO_THROW(feedHandler->unsubscribe(nullSubscriber)); // Unsubscribing null should not throw
    EXPECT_NO_THROW(feedHandler->subscribe(nullSubscriber)); // Re-subscribing null should not throw
    EXPECT_NO_THROW(feedHandler->unsubscribe(nullSubscriber));
}

TEST_F(MarketDataFeedHandlerTest, DispatchesMessagesToSubscribers) {
    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    feedHandler->subscribe(loggingSubscriber);

    MarketDataMessage msg{
        .symbol = "AAPL",
        .side = OrderSide::BUY,
        .price = 150.0,
        .quantity = 100,
        .timestamp = chrono::system_clock::now()
    };

    queue.push(std::move(msg));

    this_thread::sleep_for(100ms); // Allow some time for the message to be processed

    // No assertion here, just checking if it runs without crashing
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
}

TEST_F(MarketDataFeedHandlerTest, DoesNotCrashOnEmptyQueue) {
    // Just check that it runs without crashing
    EXPECT_NO_THROW(feedHandler->start());
    this_thread::sleep_for(100ms); // Allow some time for the dispatcher to run
    EXPECT_NO_THROW(feedHandler->stop());
}

TEST_F(MarketDataFeedHandlerTest, StopsGracefully) {
    EXPECT_NO_THROW(feedHandler->stop());
    EXPECT_NO_THROW(feedHandler->start()); // Restarting after stop should not throw
}

TEST_F(MarketDataFeedHandlerTest, EmptyQueueSubscriberDispatch) {
    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    feedHandler->subscribe(loggingSubscriber);

    // No messages in the queue, just check that it runs without crashing
    EXPECT_NO_THROW(feedHandler->start());
    this_thread::sleep_for(100ms); // Allow some time for the dispatcher to run
    EXPECT_NO_THROW(feedHandler->stop());

    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
}

TEST_F(MarketDataFeedHandlerTest, MultipleSubscribers) {
    auto loggingSubscriber1 = make_shared<LoggingSubscriber>();
    auto loggingSubscriber2 = make_shared<LoggingSubscriber>();

    feedHandler->subscribe(loggingSubscriber1);
    feedHandler->subscribe(loggingSubscriber2);

    MarketDataMessage msg{
        .symbol = "GOOGL",
        .side = OrderSide::SELL,
        .price = 2800.0,
        .quantity = 50,
        .timestamp = chrono::system_clock::now()
    };

    queue.push(std::move(msg));

    this_thread::sleep_for(100ms); // Allow some time for the messages to be processed

    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber1));
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber2));
}

TEST_F(MarketDataFeedHandlerTest, HandlesConcurrentPushes) {
    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    feedHandler->subscribe(loggingSubscriber);

    // Push multiple messages concurrently
    vector<thread> producers;
    for (int i = 0; i < 10; ++i) {
        producers.emplace_back([&]() {
            MarketDataMessage msg{
                .symbol = "MSFT",
                .side = OrderSide::BUY,
                .price = 300.0 + i,
                .quantity = 10 + i,
                .timestamp = chrono::system_clock::now()
            };
            queue.push(std::move(msg));
        });
    }

    for (auto& producer : producers) {
        producer.join();
    }

    this_thread::sleep_for(100ms); // Allow some time for the messages to be processed

    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
}

TEST_F(MarketDataFeedHandlerTest, HandlesRapidMessageDispatch) {
    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    feedHandler->subscribe(loggingSubscriber);

    // Rapidly push messages
    for (int i = 0; i < 100; ++i) {
        MarketDataMessage msg{
            .symbol = "AMZN",
            .side = OrderSide::SELL,
            .price = 3500.0 + i,
            .quantity = 5 + i,
            .timestamp = chrono::system_clock::now()
        };
        queue.push(std::move(msg));
    }

    this_thread::sleep_for(500ms); // Allow some time for the messages to be processed

    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
}

TEST_F(MarketDataFeedHandlerTest, StopsAndStartsMultipleTimes) {
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_THROW(feedHandler->start());
        this_thread::sleep_for(100ms); // Allow some time for the dispatcher to run
        EXPECT_NO_THROW(feedHandler->stop());
    }
}

TEST_F(MarketDataFeedHandlerTest, StressTestWithSimulatorOnCSVModeAndMultipleSubscribers) {
    auto loggingSubscriber1 = make_shared<LoggingSubscriber>();
    auto loggingSubscriber2 = make_shared<LoggingSubscriber>();

    feedHandler->subscribe(loggingSubscriber1);
    feedHandler->subscribe(loggingSubscriber2);

    MarketDataSimulator simulator(queue);
    simulator.setSourceType(SourceType::FILE);
    simulator.setReplayMode(ReplayMode::REALTIME);
    simulator.start();

    this_thread::sleep_for(chrono::seconds(15)); // Let it run for a while

    simulator.stop();
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber1));
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber2));
    EXPECT_NO_THROW(feedHandler->stop());
}

TEST_F(MarketDataFeedHandlerTest, StressTestWithSimulatorOnGeneratedModeAndMultipleSubscribers) {
    auto loggingSubscriber1 = make_shared<LoggingSubscriber>();
    auto loggingSubscriber2 = make_shared<LoggingSubscriber>();

    feedHandler->subscribe(loggingSubscriber1);
    feedHandler->subscribe(loggingSubscriber2);

    MarketDataSimulator simulator(queue);
    simulator.setSourceType(SourceType::GENERATED);
    simulator.setReplayMode(ReplayMode::REALTIME);
    simulator.start();

    this_thread::sleep_for(chrono::seconds(60)); // Let it run for a while

    simulator.stop();
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber1));
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber2));
    EXPECT_NO_THROW(feedHandler->stop());
}

TEST_F(MarketDataFeedHandlerTest, HandlesEarlyStopWithSimulator) {
    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    feedHandler->subscribe(loggingSubscriber);

    MarketDataSimulator simulator(queue);
    simulator.setSourceType(SourceType::GENERATED);
    simulator.setReplayMode(ReplayMode::REALTIME);
    
    EXPECT_NO_THROW(simulator.start());
    this_thread::sleep_for(chrono::seconds(5)); // Let it run for a short time
    EXPECT_NO_THROW(simulator.stop());

    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
    EXPECT_NO_THROW(feedHandler->stop());
    EXPECT_NO_THROW(feedHandler->start()); // Restarting after stop should not throw
    EXPECT_NO_THROW(feedHandler->unsubscribe(loggingSubscriber));
    EXPECT_NO_THROW(feedHandler->stop());
}
