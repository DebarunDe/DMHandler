#include "../include/MarketDataFeedHandler.h"

#include <algorithm>
#include <memory>

using namespace std;

MarketDataFeedHandler::MarketDataFeedHandler(shared_ptr<ThreadSafeMessageQueue<MarketDataMessage>> messageQueue):
    messageQueue_(messageQueue),
    statsTracker_(make_shared<MarketDataStatsTracker>()),
    running_(false)
    { }

MarketDataFeedHandler::~MarketDataFeedHandler() {
    stop();
}

const shared_ptr<MarketDataStatsTracker>& MarketDataFeedHandler::getStatsTracker() const {
    return statsTracker_;
}

void MarketDataFeedHandler::subscribe(shared_ptr<IMarketDataSubscriber> subscriber) {
    lock_guard<mutex> lock(subscriberMutex_);
    subscribers_.push_back(subscriber);
}

void MarketDataFeedHandler::unsubscribe(shared_ptr<IMarketDataSubscriber> subscriber) {
    lock_guard<mutex> lock(subscriberMutex_);
    auto it = find(subscribers_.begin(), subscribers_.end(), subscriber);
    if (it != subscribers_.end()) subscribers_.erase(it);
}

void MarketDataFeedHandler::start() {
    if (running_) return;
    running_ = true;
    dispatcherThread_ = thread(&MarketDataFeedHandler::dispatchLoop, this);
}

void MarketDataFeedHandler::stop() {
    if (!running_) return;
    running_ = false;
    if (dispatcherThread_.joinable()) dispatcherThread_.join();
}

void MarketDataFeedHandler::dispatchLoop() {
    while (running_) {
        MarketDataMessage msg;

        if (messageQueue_->tryPop(msg)) {
            lock_guard<mutex> lock(subscriberMutex_);
            statsTracker_->update(msg);
            for (const auto& sub : subscribers_) {
                if (sub) sub->onMarketData(msg);
            }
        }
        else {
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }
}