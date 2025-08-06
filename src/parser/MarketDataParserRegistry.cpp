#include "../../include/parser/MarketDataParserRegistry.h"
#include "../../include/parser/MarketDataParserFactory.h"
#include "../../include/parser/FileMarketDataParser.h"
#include "../../include/parser/GeneratedMarketDataParser.h"

using namespace std;

void registerParsers() {
    MarketDataParserFactory::registerParser("file", [] {
        return make_unique<FileMarketDataParser>();
    });

    MarketDataParserFactory::registerParser("generated", [] {
        return make_unique<GeneratedMarketDataParser>();
    });

}