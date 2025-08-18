#include "../include/MarketDataFeedHandler.h"

#include "../include/testSubscribers/LoggingSubscriber.h"
#include "../include/testSubscribers/FileLoggerSubscriber.h"
#include "../include/testSubscribers/MarketStatsDataSubscriber.h"

#include "../include/rest/MarketDataRestHandler.h"
#include "../include/parser/FileMarketDataParser.h"
#include "../include/parser/GeneratedMarketDataParser.h"
#include "../include/parser/MarketDataParserRegistry.h"

#include "../include/webSocket/IxWebSocketClient.h"
#include "../include/dataSource/FinnhubConnector.h"

#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

#include <ixwebsocket/IXWebSocket.h>

using namespace std;

// rm -rf build && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja -C build && cd build && ctest --output-on-failure && ./DMHandler && cd ..
// rm -rf build && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja -C build && cd build && ctest --output-on-failure && cd ..

atomic<bool> shutdownRequested{false};

std::thread startSignalWatcher() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);

    // Block these signals in the calling thread; child threads inherit the mask.
    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    return std::thread([set]() mutable {
        int sig = 0;
        // Wait here until SIGINT or SIGTERM arrives
        if (sigwait(&set, &sig) == 0) {
            shutdownRequested.store(true, std::memory_order_relaxed);
            // async-signal-safe write (avoid std::cout here)
            const char msg[] = "\n[INFO] Signal received. Shutting down...\n";
            (void)!write(STDERR_FILENO, msg, sizeof(msg)-1);
        }
    });
}

std::string getFinnhubApiKey() {
    const char* key = std::getenv("FINNHUB_API_KEY");
    if (key != nullptr) {
        return std::string(key);
    } else {
        std::cerr << "[WARN] FINNHUB_API_KEY not set, using dummy key\n";
        return "DUMMY_KEY";  // Or throw if you want strict behavior
    }
}

int main() {
    // Register signal handler for graceful shutdown
    auto sigThread = startSignalWatcher(); // do NOT detach yet

    // Register parsers
    registerParsers();

    // Shared queue and feed handler
    auto queue = make_shared<ThreadSafeMessageQueue<MarketDataMessage>>();
    MarketDataFeedHandler feedHandler(queue);

    // Setup subscribers
    auto loggingSub = make_shared<LoggingSubscriber>();
    auto fileLogger = make_shared<FileLoggerSubscriber>("logs/main_feed.log");
    auto statsSub = make_shared<MarketDataStatsSubscriber>(feedHandler.getStatsTracker());

    feedHandler.subscribe(loggingSub);
    feedHandler.subscribe(fileLogger);
    feedHandler.subscribe(statsSub);

    fileLogger->start();
    feedHandler.start();

    // Start REST API server
    auto restApi = make_unique<MarketDataRestApi>(feedHandler.getStatsTracker());
    restApi->start(18080);
    cout << "[INFO] REST API server running on http://localhost:18080\n";

    string apiKey = getFinnhubApiKey();
    string wsUrl = string("wss://ws.finnhub.io?token=") + apiKey;
    cout << "[INFO] Connecting to Finnhub WebSocket" << "\n";
    FinnhubConnector finnhubDataSource(
        make_unique<IxWebSocketClient>(wsUrl),
        make_unique<FinnhubMarketDataParser>(),
        queue,
        {"AAPL", "GOOGL", "MSFT", "AMZN", "TSLA", "NFLX", "NVDA", "META", "BRK.A", "V", "JPM", "UNH"}
    );

    finnhubDataSource.start();

    // Main loop — print stats every 5 seconds
    while (!shutdownRequested.load(std::memory_order_relaxed)) {
        this_thread::sleep_for(chrono::seconds(5));
        for (const auto& symbol : feedHandler.getStatsTracker()->getAllSymbols()) {
            auto stats = feedHandler.getStatsTracker()->getStats(symbol);
            cout << "[STATS] Symbol: " << symbol
                 << ", Avg Price: " << stats.getAveragePrice()
                 << ", Total Volume: " << stats.totalVolume
                 << ", Last Price: " << stats.lastPrice
                 << ", Low Price: " << stats.lowPrice
                 << ", High Price: " << stats.highPrice
                 << ", lastUpdate: " << chrono::duration_cast<chrono::seconds>(stats.lastUpdateTime.time_since_epoch()).count()
            << "\n";
        }
        cout << "_________________________________\n";
    }

    // Cleanup
    cout << "[INFO] Stopping services...\n";
    finnhubDataSource.stop();
    restApi->stop();
    feedHandler.stop();
    fileLogger->stop();

    feedHandler.unsubscribe(loggingSub);
    feedHandler.unsubscribe(fileLogger);
    feedHandler.unsubscribe(statsSub);

    cout << "[INFO] Shutdown complete.\n";
    return 0;
}