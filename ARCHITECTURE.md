# **Overview**
FinPulse is a modular market data platform designed to ingest, store, analyze, and visualize financial market data. 

The backend is organized around stable abstractions that allow new exchanges, storage backends, and analysis modules to be added with minimal changes to existing code.

**The architecture separates:**

- Transport and networking.

- Exchange-specific API logic.

- Exchange-specific message parsing.

- Canonical market-data models.

- Market-state reconstruction.

- Analytical calculations.

- Persistence and caching (Yet to implement).

The goal is to keep each layer responsible for one part of the system while allowing data to flow through the entire system without tightly coupling components together. 

# **Philosophy**
FinPulse is designed around a few core principles:

- Every component should have a single, well-defined responsibility.

- Components should be loosely coupled and communicate through stable interfaces.

- Exchange-specific logic should remain isolated from the rest of the system.

- Adding new functionality should require extending the system rather 
than modifying existing components.

- Domain models should provide a common representation of market data regardless of its source.

The long-term goal is to make FinPulse easy to extend. If a new exchange, database, visualization, or machine learning module is added months later, the existing architecture should require little to no modification.

# **Architecture**
### Canonical Domain Models

Canonical domain models represent market information independently of any particular exchange.

Examples include:

- Trade

- CandleStick

- OrderBook

- OrderBookLevel

These models are the primary data representation used by downstream systems.

Exchange specific JSON payloads should **NOT** propogate through the pipeline.

### Exchange Specific Models

Exchange specific models represent information that exists at the ingestion boundary but does not necessarily belong in the canonical domain model.

For Binance order book ingestion, these include:

- BinanceOrderBookUpdate

- BinanceOrderBookSnapshot

These models preserve Binance specific information, such as update IDs, that is required to correctly synchronize the live order book.

They are consumed by Binance specific ingestion components and are used to update canonical domain state.

# **Transport Layer**

The transport layer provides reusable networking infrastructure without containing exchange specific API logic.

### HttpClient

`HttpClient` provides generic HTTPS GET functionality.

It owns the HTTP connection boilerplate, including:

- TCP resolution.

- TCP connection.

- TLS handshake.

- HTTP request construction.

- HTTP response handling.

- TLS shutdown.

`HttpClient` does not know anything about Binance endpoints, symbols, order books, or JSON formats.

This allows the same HTTP infrastructure to be reused by future exchange adapters and other services.

### WebSocket Transport

WebSocket communication is currently implemented using Boost.Beast and Boost.Asio

Unlike `HttpClient`, WebSocket communication is currently owned by the exchange client because the client must coordinate the exchange specific streaming endpoint and incoming market data messages (Will make a generic WebSocket in the future).

The current implementation is synchronous.

Migration to asynchronous Boost.Asio operations is planned for a later stage.

# **Exchange Adapters**

Exchange adapters contain logic specific to communicating with a particular market data provider.

The current exchange implementation is Binance (will add more soon).

## BinanceClient

`BinanceClient` coordinates Binance's live market data ingestion.

Its responsibilities include:

- Establishing and closing the Binance WebSocket connection.

- Performing the WebSocket handshake.

- Reading WebSocket messages.

- Passing raw messages to `BinanceParser`.

- Passing order book updates through `OrderBookReconstructor`.

- Triggering snapshot recovery when a sequence gap is detected.

- Detecting synchronization failures.

`BinanceClient` does not own generic HTTP communication.

REST operations are delegated to dedicated Binance components using the shared `HttpClient`.

The client therefore acts primarily as an orchestration layer between Binance's streaming API and FInPulse's market data components.

# **BinanceOrderBookSnapshotRetriever**

`BinanceOrderBookSnapshotRetriever` is responsible specifically for retrieving Binance order book snapshots.

it: 

1. Constructs the Binance depth endpoints.

2. Uses `HttpClient` to perform the HTTPS request.

3. Passes the returned JSON to `BinanceParser`.

4. Returns a `BinanceOrderBookSnapshot`.

It does not implement HTTP connection logic itself.

# **Parsing Layer**
### BinanceParser

`BinanceParser` converts Binance specific JSON messages into structured models.

It currently handles:

- Binance trade messages.

- Binance incremental order book updates.

- Binance order book snapshots.

The parser is responsible for interpreting Binance's JSON schema but is not responsible for networking or maintaining market state.

This creates a clean boundary between external payloads and internal application logic. 

# **Order Book State Management**
### OrderBookReconstructor

`OrderBookReconstructor` maintains the current state of a canonical `OrderBook`.

It does not communicate with Binance directly.

Its responsibilities are:

- Initializing the book from a snapshot

- Applying incremental updates.

- Ignoring updates that have already been processed.

- Detecting update-ID gaps.

- Removing price levels whose quantity becomes zero.

- Updating existing price levels.

- Adding new price levels.

- Producing the current canonical `OrderBook`.

### Initialization

A REST snapshot establishes the intial state of the order book.

The snapshot's `last_update_id` becomes the starting sequence position of the reconstructed book.

### Incremental Updates

After initialization, Binance WebSocket updates are applied incrementally.

Each update contains:

- First update ID.

- Final update ID.

- Bid changes.

- Ask changes.

- Symbol.

- Event timestamp.

The reconstructor verifies that the update sequence is compatible with the current state.

Updates are classified as follows:

- `Applied`

- `Ignored`

- `SequenceGap`

`Ignored` updates are older than or already covered by the current state.

`Applied` updates safely modify the current book.

`SequenceGap` indicates that the current reconstructed state can no longer be trusted and requires recovery.

# **Order Book Recovery**

When `OrderBookReconstructor` reports a `SequenceGap`, the Binance client requests a new REST snapshot.

The recovery process goes as follows:

1. Detect the sequence gap.

2. Request a fresh Binance order book snapshot.

3. Parse the snapshot

4. Reinitialize `OrderBookReconstructor`.

5. Resume processing WebSocket updates.

This allows the live order book to recover from missed or invalidated update sequences without restarting the entire client.

The recovery mechanism has been tested against live Binance market data. The reconstructed order book successfully initialized from a REST snapshot and continued applying subsequent WebSocket depth updates.

# **Analysis**

FinPulse separates market data from derived analysis.

Canonical domain models represent facts about the market and remain independent of analytical calculations.

Analysis modules consume these models and produce derived metrics without modifying the underlying market data.

For example, candle analysis separated into:

- `CandleMetrics`: structural properties of a candle.

- `ATR`: a stateful streaming volatility calculation.

- `VolatilityMetrics`: volatility measurements derived from candle metrics and ATR.

This separation allows analytical modules to evolve independently from the canonical market data modules and makes it possible to add new indicators or machine learning features without changing the underlying market data representation.

# **Streaming Analysis**

FinPulse performs certain analytical calculations incrementally as market data arrives rather than repeatedly recalculating them from historical data.

ATR is implemented as a stateful streaming calculation. 

After initialization, each new candle requires only:

- The previous ATR.

- The new True Range.

- The previous candle's close.

This allows ATR to be updated in constant time per candle while avoiding unnecessary historical scans.

# **Data Flow**

FinPulse currently supports two primary paths for Binance order book data:

### REST Snapshot

The REST path obtains an initial or recovery snapshot of the order book.

`HttpClient` handles generic HTTPS communication, while Binance specific components handle the endpoint construction and response parsing.

### Live WebSocket

The Websocket path receives incremental order book updates.

`BinanceClient` manages the connection and coordinates the parser and order book reconstructor.

### Synchronization

The REST snapshot and WebSocket stream work together to maintain a synchronized order book. 

A sequence gap invalidates the current state and triggers snapshot recovery.

    ### **Architecture diagrams will be added later once the backend architecture and storage pipelines are finalized.

# ***Storage**
Storage is responsible for persistence, caching, and database communication.

The storage layer is intentionally separated from ingestion.

Ingestion components should produce canonical market data models rather than directly depending on a particular database implementation.

Future storage components may consume these models for:

- Historical market data storage.

- Real time caching.

- Market data replay.

- Analytical queries.

The specific database and caching architecture will be documented once those components are implemented.

# **Project Structure**

The backend is organized into separate `include/` and `src/` trees.

`include/` contains header files and defines the interfaces, models, and class declarations used throughout the backend.

`src/` contains implementations for components that require a corresponding source file. Not every header has an implementation file; some headers contain definitions that do not require a separate `.cpp` file.

The backend also contains a `build/` directory generated by CMake and used for compiled build artifacts.

## **backend/include**

### **`analysis/`**

Contains analysis components that operate on canonical market-data models.

* `candle_metrics_calculator.hpp`
* `volatility_metrics_calculator.hpp`

### **`analysis/volatility/`**

Contains stateful volatility calculations.

* `atr.hpp`

### **`common/`**

Contains reusable infrastructure shared across the backend.

* `config.hpp`
* `logger.hpp`
* `http_client.hpp`

### **`ingestion/`**

Contains market-data ingestion abstractions and exchange-specific ingestion components.

* `market_ingestor.hpp`

### **`ingestion/binance/`**

Contains Binance-specific market-data ingestion components.

* `binance_client.hpp`
* `binance_parser.hpp`
* `order_book_reconstructor.hpp`
* `snapshot_retriever.hpp`

### **`ingestion/binance/updates/`**

Contains Binance-specific models used to represent streaming and snapshot update data.

* `binance_order_book_snapshot.hpp`
* `binance_order_book_updates.hpp`
* `update_results.hpp`

### **`models/`**

Contains canonical domain models used throughout FinPulse.

* `candle_stick.hpp`
* `trade.hpp`

### **`models/metrics/`**

Contains canonical models representing derived market metrics.

* `candle_stick_metrics.hpp`
* `volatility_metrics.hpp`

### **`models/order_book/`**

Contains canonical order-book models.

* `order_book_level.hpp`
* `order_book.hpp`

### **`storage/`**

Contains storage abstractions and database interfaces.

* `database.hpp`

The storage layer is currently under development.

---

## **backend/src**

The `src/` directory contains implementations for backend components that require separate compilation units.

### **`analysis/`**

Contains implementations for analysis components.

* `candle_metrics_calculator.cpp`
* `volatility_metrics_calculator.cpp`

### **`analysis/volatility/`**

Contains implementations for stateful volatility calculations.

* `atr.cpp`

### **`common/`**

Contains implementations for reusable common infrastructure.

* `http_client.cpp`
* `logger.cpp`

`logger.cpp` currently exists as an implementation placeholder while the logging system is being developed.

### **`ingestion/`**

Contains implementations for market-data ingestion components.

### **`ingestion/binance/`**

Contains Binance-specific implementations.

* `binance_client.cpp`
* `binance_parser.cpp`
* `order_book_reconstructor.cpp`
* `snapshot_retriever.cpp`

### **`storage/`**

Contains storage implementations.

* `database.cpp`

The database implementation is currently a placeholder while the storage layer is being developed.

### **`main.cpp`**

The application entry point.

`main.cpp` currently initializes and starts the exchange-ingestion layer. Binance is currently the only implemented exchange.

---

## **backend/build**

The `build/` directory is generated by the CMake build process and contains compiled artifacts and generated build files.

It is not part of the application's source architecture.

---

## **Current Backend Organization**

The current backend therefore follows this general separation:

* **`common/`** — reusable infrastructure.
* **`ingestion/`** — external market-data acquisition and exchange-specific logic.
* **`models/`** — canonical market-data representations.
* **`analysis/`** — calculations and derived metrics.
* **`storage/`** — persistence infrastructure.
* **`main.cpp`** — application startup and orchestration.

The `include/` and `src/` trees mirror this organization where separate implementations are required.

# **Design Decisions**
### Canonical Market Data

All exchanges can represent the same market information differently.

FinPulse converts exchange-specific representations into canonical domain models so downstream systems remain exchange-independent.

### MarketIngestor Interface

`MarketIngestor` defines the common interface that exchange-specific ingestion clients are expected to implement.

This allows new exchanges to be added without requiring changes to storage, visualization, replay, or analysis modules.

### Exchange Isolation

Exchange specific networking, API endpoints, JSON schemas, and synchronization rules remain inside exchange specific ingestion components. 

Reason being is because trying to make one generic component for all exchanges will lead to a fruitless endevour, where it'll created a cluttered and too far abstracted work environment. 

Downstream systems operate on canonical models rather than exchange payloads. This simplifies the sharing of data between components and infrastructure.

### Generic Transport Infrastructure

Generic networking functionality belongs in reusable infrastructure such as `HttpClient`.

Exchange components should configure and consume this infrastructure rather than reimplementing HTTPS/TLS connection boilerplate.

Making it easier to maintain and avoid writing boiler plate code repeatedly.

### Single responsibility

As established in the philosphy section, components should perform one primary responsibility.

For example:

- `HttpClient` handles HTTP transport.

- `BinanceOrderBookSnapshotRetriever` handles Binance snapshot retrieval.

- 'BinanceParser` handles Binance payload parsing.

- `OrderBookReconstructor` handles order book state.

- `BinanceClient` coordinates the live binance ingestion process.

This prevents exchange clients from becoming large collections of unrelated networking, parsing, state management, and persistence logic.

### Boost.Beast

Boost.Beast was selected for WebSocket communication due to its native integration with Boost.Asio and its mature implementation.

### Boost.Asio

Boost.Asio provides the underlying networking primitives used by FinPulse's HTTP and WebSocket infrastructure.

The current Binance WebSocket implementation is synchronous, with asynchronous networking planned for future development.

### Conan

Conan was selected as the package manager for FinPulse because it provides reproducible dependency management, consistent library versions across development environments, and straightforward integration with CMake.
