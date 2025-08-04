#include "../include/MarketDataFeedHandler.h"

#include "../include/testSubscribers/LoggingSubscriber.h"
#include "../include/testSubscribers/FileLoggerSubscriber.h"
#include "../include/testSubscribers/MarketStatsDataSubscriber.h"

#include "../include/rest/MarketDataRestHandler.h"

#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

using namespace std;

// rm -rf build && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja -C build && cd build && ctest --output-on-failure && ./DMHandler && cd ..

atomic<bool> shutdownRequested{false};

void signalHandler(int signum) {
    shutdownRequested = true;
    cout << "\n[INFO] Shutdown signal received. Terminating...\n";
}

int main() {
    // Register signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Shared queue and feed handler
    ThreadSafeMessageQueue<MarketDataMessage> queue;
    MarketDataFeedHandler feedHandler(queue);

    // Setup subscribers
    auto loggingSub = make_shared<LoggingSubscriber>();
    auto fileLogger = make_shared<FileLoggerSubscriber>("logs/main_feed.log");
    auto statsTracker = make_shared<MarketDataStatsTracker>();
    auto statsSub = make_shared<MarketDataStatsSubscriber>(statsTracker);

    feedHandler.subscribe(loggingSub);
    feedHandler.subscribe(fileLogger);
    feedHandler.subscribe(statsSub);

    fileLogger->start();
    feedHandler.start();

    // Start REST API server
    auto restApi = make_unique<MarketDataRestApi>(statsTracker);
    restApi->start(18080);
    cout << "[INFO] REST API server running on http://localhost:18080\n";

    // Start simulator: phase 1 — from file
    MarketDataSimulator simulator(queue);
    simulator.setSourceType(SourceType::FILE);
    simulator.setReplayMode(ReplayMode::REALTIME);
    simulator.start();

    cout << "[INFO] Phase 1: FILE source, REALTIME replay\n";
    this_thread::sleep_for(chrono::seconds(15));
    simulator.stop();

    // Phase 2 — generated data, accelerated
    simulator.setSourceType(SourceType::GENERATED);
    simulator.setReplayMode(ReplayMode::ACCELERATED, 5.0);
    simulator.start();

    cout << "[INFO] Phase 2: GENERATED source, ACCELERATED replay\n";
    while (!shutdownRequested) {
        this_thread::sleep_for(chrono::seconds(1));
    }

    // Cleanup
    simulator.stop();
    restApi->stop();
    feedHandler.stop();
    fileLogger->stop();

    feedHandler.unsubscribe(loggingSub);
    feedHandler.unsubscribe(fileLogger);
    feedHandler.unsubscribe(statsSub);

    cout << "[INFO] Shutdown complete.\n";
    return 0;
}