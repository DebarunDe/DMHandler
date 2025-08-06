#include "../../include/parser/MarketDataParserFactory.h"
#include "../../include/MarketDataMessage.h"
#include "../../include/OrderSide.h"

using namespace std;

unordered_map<string, CreatorFunction>& MarketDataParserFactory::getRegistry() {
    static unordered_map<string, CreatorFunction> registry;
    return registry;
}

void MarketDataParserFactory::registerParser(const string& name, CreatorFunction creator) {
    getRegistry()[name] = creator;
}

unique_ptr<MarketDataParser> MarketDataParserFactory::create(const string& name) {
    const auto& registry = getRegistry();

    auto it = registry.find(name);
    if (it == registry.end()) throw runtime_error("Parser type not registered: " + name);
    return it->second();
}
