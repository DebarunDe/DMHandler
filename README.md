# DMHandler
Market Data Feed Handler from the Finnhub API.

## DMHandler
DMHandler is a robust market data feed handler designed to process, simulate, and analyze financial market data. It integrates with the Finnhub API to fetch real-time trade data and provides a modular architecture for simulating, parsing, and handling market data. The project is built with extensibility and testing in mind, allowing developers to simulate market conditions, parse data from various sources, and expose processed data to consumers via a pub-sub model and REST API.

---

## Simulator
The `MarketDataSimulator` is the core component responsible for simulating market data. It supports two modes:
1. **File-based Simulation**: Reads market data from a CSV file and replays it.
2. **Generated Simulation**: Uses the `MarketDataGenerator` to create mock market data dynamically.

### Logic and Reasoning
The simulator is designed to:
- Replay market data in real-time, accelerated, or fixed-delay modes.
- Provide a controlled environment for testing and debugging.
- Allow seamless switching between static (file-based) and dynamic (generated) data sources.

The simulator uses multithreading to ensure data is emitted at the desired rate without blocking the main thread.

---

## Generator
The `MarketDataGenerator` is used to create mock market data for testing purposes. It generates realistic trade data, including:
- Symbols (e.g., AAPL, TSLA).
- Prices with configurable volatility.
- Quantities within a specified range.
- Randomized timestamps.

### Why It Is Used
The generator is essential for testing the `MarketDataSimulator` in scenarios where real market data is unavailable or impractical to use. It ensures that the simulator can operate in a controlled environment with predictable data.

---

## Parser
The `MarketDataParser` folder implements a factory architecture to handle parsing of market data from various sources. The parser is designed to be extensible, allowing new data formats to be added easily.

### Current Parsers
#### 1. **FileMarketDataParser**
- **Source**: CSV files.
- **Parsing Method**: Reads lines from a CSV file, splits them into fields, and converts them into `MarketDataMessage` objects.

#### 2. **FinnhubMarketDataParser**
- **Source**: WebSocket streams.
- **Parsing Method**: Processes JSON payloads received from the Finnhub WebSocket API and converts them into `MarketDataMessage` objects.

#### 3. **GeneratedMarketDataParser**
- **Source**: Generated Data.
- **Parsing Method**: Creates `MarketDataMessage` objects.

---

## Datasource
The `FinnhubConnector` is responsible for connecting to the Finnhub API to fetch real-time trade data.

### How It Works
1. **API Endpoint**: The Finnhub API WebSocket endpoint is used to subscribe to real-time trade data. [Finnhub API Trades Endpoint](https://finnhub.io/docs/api/websocket-trades).
2. **WebSocket Connection**: The connector establishes a WebSocket connection and subscribes to trade data for specified symbols.
3. **Data Processing**: Incoming JSON messages are parsed and converted into `MarketDataMessage` objects.

---

## WebSocket
The project uses `IxWebSocket` as a wrapper around the WebSocket implementation. The `IxWebSocket` library provides:
- A simple interface for establishing WebSocket connections.
- Automatic reconnection in case of network failures.
- Thread-safe message handling.

The `IxWebSocket` wrapper is used in the `FinnhubConnector` to manage the WebSocket connection and handle incoming messages.

---

## Feed Handler
The `FeedHandler` is responsible for managing the flow of market data. It implements a pub-sub model to expose data to consumer threads. Key features include:
- **Publisher**: The feed handler acts as a publisher, emitting market data to subscribed consumers.
- **Subscribers**: Consumers can subscribe to specific symbols or data streams.

The pub-sub model ensures that data is distributed efficiently to all interested consumers.

---

## Subscribers
The project includes several test subscribers to demonstrate the functionality of the feed handler.

### Test Subscribers
#### 1. **ConsoleSubscriber**
- Logs market data to the console for debugging purposes.

#### 2. **FileSubscriber**
- Writes market data to a file for offline analysis.

#### 3. **StatsSubscriber**
- Aggregates market data and calculates statistics (e.g., average price, total volume).

Each subscriber demonstrates a unique use case for consuming market data.

---

## StatsTracker
The `MarketDataStatsTracker` processes market data and exposes aggregated statistics. Key features include:
- **Data Aggregation**: Tracks metrics such as average price, total volume, and trade count.

---

## API
The `MarketDataRestHandler` provides a REST API to expose market data and statistics. Key endpoints include:
- **GET /stats**: Returns aggregated market data statistics.
- **GET /data**: Returns the latest market data for subscribed symbols.

The REST API is built using a lightweight HTTP server and is designed for high performance.

---

## Main
The `main.cpp` file demonstrates the functionality of the project. It:
1. Initializes the `MarketDataSimulator` and `FeedHandler`.
2. Starts the simulator in either file-based or generated mode.
3. Subscribes test consumers to the feed handler.
4. Starts the REST API to expose data and statistics.

The `main.cpp` file serves as the entry point for the project and ties all components together.

---

## Dependencies
The project uses the following third-party libraries:
1. **IxWebSocket**: For WebSocket communication.
2. **Crow**: For IxWebSocket and API support.
3. **Asio**: For Crow and IxWebSocket support.
3. **nlohmann/json**: For JSON parsing.

These libraries are included in the `third_party` folder and are managed via CMake.

---

## Infrastructure
The `infrastructure` folder contains scripts and configurations for deploying the project. Key features include:
- **Binary Upload**: Automates the upload of compiled binaries to an S3 bucket for distribution.

---

## Status
![Build Status](https://github.com/your-repo/actions/workflows/build.yml/dmhandler-build-test.svg)
![Infra Status](https://github.com/your-repo/actions/workflows/test.yml/dmhandler-deploy-infrastructure.svg)

---

## How to Build and Run
1. **Clone the Repository**:
   ```bash
   git clone https://github.com/your-repo/DMHandler.git
   cd DMHandler

