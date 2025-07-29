#include "../include/MarketDataFeedHandler.h"

#include "../include/testSubscribers/LoggingSubscriber.h"

#include <iostream>

using namespace std;

int main() {
    // cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja -C build
    // cd build && ctest --output-on-failure && ./DMHandler
    
    ThreadSafeMessageQueue<MarketDataMessage> queue;
    MarketDataFeedHandler feedHandler(queue);

    auto loggingSubscriber = make_shared<LoggingSubscriber>();
    feedHandler.subscribe(loggingSubscriber);
    feedHandler.start();

    MarketDataSimulator simulator(queue);
    simulator.setSourceType(SourceType::FILE);
    simulator.setReplayMode(ReplayMode::REALTIME);
    simulator.start();
    this_thread::sleep_for(chrono::seconds(15));
    simulator.stop();

    simulator.setSourceType(SourceType::GENERATED);
    simulator.start();
    this_thread::sleep_for(chrono::seconds(60));
    simulator.stop();

    feedHandler.stop();
    feedHandler.unsubscribe(loggingSubscriber);


    return 0;
}