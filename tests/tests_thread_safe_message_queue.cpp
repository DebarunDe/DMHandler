#include <gtest/gtest.h>
#include "../include/ThreadSafeMessageQueue.h"
#include <thread>
#include <chrono>
#include <random>

using namespace std;

TEST(ThreadSafeMessageQueueTest, DefaultConstructor) {
    ThreadSafeMessageQueue<int> queue;
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST(ThreadSafeMessageQueueTest, PushAndPop) {
    ThreadSafeMessageQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);

    int testlVal = 4;
    queue.push(testlVal);

    int testrVal = 5;
    queue.push(std::move(testrVal));

    EXPECT_EQ(queue.size(), 5);
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.pop(), 1);
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_EQ(queue.pop(), 3);
    EXPECT_EQ(queue.pop(), testlVal);
    EXPECT_EQ(queue.pop(), 5);
    EXPECT_TRUE(queue.empty());
}

TEST(ThreadSafeMessageQueue, SizeandTop) {
    ThreadSafeMessageQueue<int> queue;
    EXPECT_EQ(queue.size(), 0);

    queue.push(1);
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.top(), 1);

    queue.push(2);
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.top(), 1);

    queue.pop();
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.top(), 2);

    queue.push(3);
    EXPECT_EQ(queue.size(), 2);
    EXPECT_EQ(queue.top(), 2);

    queue.pop();
    EXPECT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.top(), 3);

    queue.pop();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_THROW(queue.top(), runtime_error);
} 

TEST(ThreadSafeMessageQueue, Clear) {
    ThreadSafeMessageQueue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.size(), 3);
    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_THROW(queue.top(), runtime_error);
}

TEST(ThreadSafeMessageQueue, PopWaits) {
    ThreadSafeMessageQueue<int> queue;

    thread producer([&queue]() {
        this_thread::sleep_for(chrono::milliseconds(100));
        queue.push(1);
    });

    thread consumer([&queue]() {
        int value = queue.pop();
        EXPECT_EQ(value, 1);
        EXPECT_TRUE(queue.empty());
    });

    producer.join();
    consumer.join();

    EXPECT_TRUE(queue.empty());
}

TEST(ThreadSafeMessageQueue, StressTest) {
    ThreadSafeMessageQueue<int> queue;

    const int numThreads = 20;
    const int numMessages = 1000;
    const int totalMessages = numThreads * numMessages;

    std::atomic<int> totalPopped{0};
    vector<thread> producers;
    vector<thread> consumers;

    auto jitter = []() {
        static thread_local mt19937 rng(random_device{}());
        uniform_int_distribution<int> dist(0, 5);  // 0–5ms
        this_thread::sleep_for(chrono::milliseconds(dist(rng)));
    };

    // Producers
    for (int i = 0; i < numThreads; ++i) {
        producers.emplace_back([&queue, i, &jitter]() {
            for (int j = 0; j < numMessages; ++j) {
                jitter();  // simulate inconsistent delivery
                queue.push(i * numMessages + j);  // still unique values
            }
        });
    }

    // Consumers
    for (int i = 0; i < numThreads; ++i) {
        consumers.emplace_back([&queue, &totalPopped, &jitter]() {
            for (int j = 0; j < numMessages; ++j) {
                jitter();  // simulate inconsistent delivery
                int value = queue.pop();
                (void)value;  // we're not validating exact value anymore
                ++totalPopped;
            }
        });
    }

    for (auto& producer : producers) producer.join();
    for (auto& consumer : consumers) consumer.join();

    EXPECT_EQ(totalPopped.load(), totalMessages);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    EXPECT_THROW(queue.top(), runtime_error); 
}