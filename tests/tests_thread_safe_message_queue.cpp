#include <gtest/gtest.h>
#include "../include/ThreadSafeMessageQueue.h"
#include <thread>
#include <chrono>

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

TEST(ThreadSafeMessageQueue, PushAndPopMultipleThreads) {
    ThreadSafeMessageQueue<int> queue;

    const int numThreads = 10;
    vector<thread> producers;
    vector<thread> consumers;

    for (int i = 0; i < numThreads; ++i) {
        producers.emplace_back([&queue, i]() {
            for (int j = 0; j < 10; ++j) {
                queue.push(i * 10 + j);
            }
        });

        consumers.emplace_back([&queue, i]() {
            for (int j = 0; j < 10; ++j) {
                int value = queue.pop();
                EXPECT_GE(value, i * 10);
                EXPECT_LT(value, (i + 1) * 10);
            }
        });
    }

    for (auto& producer : producers) producer.join();
    for (auto& consumer : consumers) consumer.join();

    EXPECT_TRUE(queue.empty());
}