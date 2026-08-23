# **FinPulse Architecture**

## **Overview**

FinPulse is a modular market data platform designed to ingest, store, analyze, and visualize financial market data.

The backend is organized around stable abstractions that allow new exchanges, storage backends, and analysis modules to be added with minimal changes to existing code.

The architecture separates:

* Transport and networking.
* Exchange-specific API logic.
* Exchange-specific message parsing.
* Canonical market-data models.
* Market-state reconstruction.
* Analytical calculations.
* Persistence and caching.

The goal is to keep each layer responsible for one part of the system while allowing data to flow through the entire system without tightly coupling components. This principle is applied within reason; some components like `OrderBookReconstructor` and `BinanceParser`, inherently depend on exchange-specific behavior and therefore cannot be made fully generic without introducing unnecessary abstraction.

---

# **Philosophy**

FinPulse is designed around a few core principles:

* Every component should have a single, well-defined responsibility.

* Components should be loosely coupled and communicate through stable interfaces.

* Exchange-specific logic should remain isolated from the rest of the system.

* Generic infrastructure should be reusable across exchanges and services.

* Adding new functionality should require extending the system rather than modifying existing components whenever possible.

* Domain models should provide a common representation of market data regardless of its source.

* Transport, parsing, state management, analysis, and storage should remain separate concerns.

The long-term goal is to make FinPulse easy to extend. If a new exchange, database, visualization, or machine-learning module is added months later, the existing architecture should require little to no modification.

---

# **Architecture**

## **Canonical Domain Models**

Canonical domain models represent market information independently of any particular exchange.

Examples include:

* `Trade`
* `CandleStick`
* `OrderBook`
* `OrderBookLevel`

These models are the primary data representation used by downstream systems.

Exchange-specific JSON payloads should not propagate through the rest of the application.

---

## **Exchange-Specific Models**

Exchange-specific models represent information that exists at the ingestion boundary but does not necessarily belong in the canonical domain model.

For Binance order-book ingestion, these include:

* `BinanceOrderBookUpdate`
* `BinanceOrderBookSnapshot`

These models preserve Binance-specific information, such as update IDs, that is required to correctly synchronize the live order book.

They are consumed by Binance-specific ingestion components and are used to update canonical domain state.

---

## **Canonical Model Pipelines**

Each canonical model represents a specific type of market data used throughout FinPulse.

Canonical models are not necessarily created or processed in the same way. Some models can be created directly from parsed exchange data, while others require additional components to aggregate, reconstruct, or calculate information before they can be fully utilized.

The following sections describe the components currently associated with each canonical model and how the data flows through FinPulse.

### **Trade**

`Trade` represents an individual executed market trade.

Trade is currently the simplest canonical model in FinPulse. Binance trade messages are parsed directly into the `Trade` model by `BinanceParser`.

**Required components:**

* `BinanceParser`

**Downstream components:**

* `CandleAggregator` will consume trades to construct `CandleStick` models (PLANNED).
* `Database` will persist trades for historical market-data storage.
* Future analysis components may consume trades directly.

**Pipeline:**

`Binance JSON → BinanceParser → Trade → CandleAggregator / Database / Analysis`

---

### **CandleStick**

`CandleStick` represents aggregated market activity over a defined time interval.

Unlike `Trade`, candles are not currently parsed directly from Binance. `CandleAggregator` consumes canonical `Trade` objects and incrementally constructs one in-progress candle for the configured timeframe.

**Required components:**

* `CandleAggregator` 

**Candle Aggregation**

`CandleAggregator` operates exclusively on the canonical `Trade` model and therefore does not contain exchange specific logic. A single aggregator implementation can be shared by all exchanges that produce canonical `Trade` objects.

The aggregator is configured with a timeframe, such as one minute, and it processes trades sequentially.

The aggregator maintains the current candle internally. Each incoming trade is used to update:

For each trade, the aggregator:

* Determines which candle interval the trade belongs to.
* Creates a new candle when no active candle exists.
* Uses the first trade's price as the candle's open price
* Updates the candle's close price with each subsequent trade.
* Updates the high price when a trade occurs above the current highest price.
* Updates the low price when a trade occurs below the current lowest price.
* Adds each trade's quantity to the candle's volume.
* Assigns the candle's timestamp to the beginning of its timeframe interval.

When a trade belongs to a new timeframe, the current candle is completed and returned. A new candle is then created for incoming trade. 

The aggregator therefore maintains only the current active candle rather than modifying previously completed candles.

These are the values the aggregator works with:

* Open price — the price of the first trade in the interval.
* High price — the highest trade price observed during the interval.
* Low price — the lowest trade price observed during the interval.
* Close price — the price of the most recent trade.
* Volume — the accumulated quantity of all trades in the interval.
* Timestamp — the beginning of the candle's time interval.

**Required components:**

* `Trade`
* `CandleAggregator`
* `CandleStick`

**Pipeline:**

`Trade → CandleAggregator → CandleStick`

**Timeframe:**

The timeframe belongs to `CandleAggregator`, not the `CandleStick` model.

For example, if the aggregator is configured for a one-minute timeframe, a candle timestamp of `14:32:00` represents the interval:

`[14:32:00, 14:33:00)`

The candle itself does not need to store its timeframe because the timeframe is a property of the aggregation process.

**Candle Lifecycle:**

A candle begins when the first trade belonging to its timeframe is received.

For example:
Trade @ 14:32:05 → Create 14:32 candle 
Trade @ 14:32:17 → Update 14:32 candle 
Trade @ 14:32:41 → Update 14:32 candle 
Trade @ 14:32:59 → Update 14:32 candle 
Trade @ 14:33:02 → Complete 14:32 candle 
                ↓ Create 14:33 candle   (**Will provide a better diagram later**)

Completed candle are passed downstream to analysis and storage components.

`CandleAggregator` does not calculate analytical metrics and not persist candles.

**Candle Analysis**

Completed `CandleStick` models are consumed by several analysis components.

`CandleMetricsCalculator` receives a completed `CandleStick` and calculates its structural properties, represented by `CandleMetrics`.

`CandleMetrics` describes the geometry of the candle:

* Range.
* Body.
* Upper wick.
* Lower wick.

The resulting `CandleMetrics` can then be passed to `VolatilityMetricsCalculator` alongside the current ATR value to produce `VolatilityMetrics`.

**Volatility**

`ATR` is a stateful streaming calculation that consumes completed `CandleStick` models.

ATR maintains its own state and calculates True Range using the current and previous candle. Once enough candles have been processed for the configured period, ATR produces the current Average True Range.

`VolatilityMetricsCalculator` combines the current ATR with `CandleMetrics` to produce:

* ATR.
* Range relative to ATR.
* Body relative to ATR.

**Required components:**

* `CandleAggregator` 
* `CandleMetricsCalculator`
* `ATR`
* `VolatilityMetricsCalculator`

**Analysis pipeline**

`CandleStick → CandleMetricsCalculator → CandleMetrics`

`CandleStick → ATR`

`CandleMetrics + ATR → VolatilityMetricsCalculator → VolatilityMetrics`

**Storage:**

* `Database` will persist completed candles for historical analysis.

*Note: `CandleAggregator` does not interact directly with the database*

**Pipeline:**

`Trade → CandleAggregator → CandleStick → CandleMetricsCalculator → CandleMetrics`

`CandleStick → ATR`

`CandleMetrics + ATR → VolatilityMetricsCalculator → VolatilityMetrics`

---

### **OrderBook**

`OrderBook` represents the current reconstructed state of an order book.

Unlike `Trade`, the canonical `OrderBook` is not produced directly by `BinanceParser`.

Binance provides order-book data through an initial REST snapshot and incremental WebSocket updates. These are represented by exchange-specific models and passed to `OrderBookReconstructor`.

`OrderBookReconstructor` maintains the current order-book state and constructs the canonical `OrderBook` when requested.

**Canonical Components:**

* `OrderBook`
* `OrderBookLevel`
* `OrderBookReconstructor`

`OrderBook` contains:

* Symbol.
* Bid levels.
* Ask levels.
* Timestamp.

Each bid and ask is represented by an `OrderBookLevel`, which contains:

* Price.
* Quantity.

---

### **Binance Order Book Models**

The order-book reconstruction process requires several Binance-specific models.

#### **BinanceOrderBookSnapshot**

`BinanceOrderBookSnapshot` represents the initial order-book state retrieved from Binance's REST API.

It contains:

* Symbol.
* Last update ID.
* Bid levels.
* Ask levels.

`BinanceOrderBookSnapshotRetriever` retrieves the snapshot using `HttpClient` and passes the response to `BinanceParser` for parsing.

---

#### **BinanceOrderBookUpdate**

`BinanceOrderBookUpdate` represents an incremental change to the existing Binance order book.

Instead of repeatedly retrieving complete order books, Binance provides updates containing only the price levels that have changed.

The update contains:

* Symbol.
* First update ID.
* Final update ID.
* Timestamp.
* Bid changes.
* Ask changes.

`BinanceParser` converts incoming WebSocket messages into `BinanceOrderBookUpdate` models.

---

### **OrderBook Reconstruction**

`OrderBookReconstructor` is responsible for combining the snapshot and incremental updates into the canonical `OrderBook`.

Its responsibilities include:

* Initializing the order book from a snapshot.
* Applying incremental updates.
* Tracking the latest update ID.
* Ignoring updates that have already been processed.
* Detecting sequence gaps.
* Removing price levels whose quantity becomes zero.
* Updating existing price levels.
* Adding new price levels.
* Producing the current canonical `OrderBook`.

`UpdateResult` provides the result of applying an update:

* `Applied`
* `Ignored`
* `SequenceGap`

A `SequenceGap` indicates that the current reconstructed state can no longer be trusted and requires snapshot recovery.

---

### **Order Book Snapshot Retrieval**

`BinanceOrderBookSnapshotRetriever` is responsible for retrieving the initial or recovery snapshot required by `OrderBookReconstructor`.

It uses:

* `HttpClient` for generic HTTPS communication.
* `BinanceParser` for parsing the Binance response.
* `BinanceOrderBookSnapshot` for representing the exchange-specific snapshot.

The snapshot retriever therefore connects the generic transport layer to the Binance-specific order-book reconstruction process without placing HTTP logic inside `OrderBookReconstructor`.

**Order Book Pipeline:**

`BinanceClient → BinanceOrderBookSnapshotRetriever → BinanceParser → BinanceOrderBookSnapshot → OrderBookReconstructor`

After initialization:

`Binance WebSocket → BinanceParser → BinanceOrderBookUpdate → OrderBookReconstructor → OrderBook`

The resulting `OrderBook` represents the current reconstructed market state and is intended to be used by live-state components such as Redis.

---

# **Transport Layer**

The transport layer provides reusable networking infrastructure without containing exchange-specific API logic.

## **HttpClient**

`HttpClient` provides generic HTTPS GET functionality.

It owns the HTTP connection boilerplate, including:

* TCP resolution.
* TCP connection.
* TLS handshake.
* HTTP request construction.
* HTTP response handling.
* TLS shutdown.

`HttpClient` does not know anything about Binance endpoints, symbols, order books, or JSON formats.

This allows the same HTTP infrastructure to be reused by future exchange adapters and other services.

---

## **WebSocket Transport**

WebSocket communication is currently implemented using Boost.Beast and Boost.Asio.

Unlike `HttpClient`, WebSocket communication is currently owned by the exchange client because the client must coordinate the exchange-specific streaming endpoint and incoming market-data messages.

A generic WebSocket abstraction may be added in the future if multiple exchanges require the same functionality.

The current implementation is synchronous.

Migration to asynchronous Boost.Asio operations is planned for a later stage.

---

# **Exchange Adapters**

Exchange adapters contain logic specific to communicating with a particular market-data provider.

The current exchange implementation is Binance.

## **BinanceClient**

`BinanceClient` coordinates Binance's live market-data ingestion.

Its responsibilities include:

* Establishing and closing the Binance WebSocket connection.
* Performing the WebSocket handshake.
* Reading WebSocket messages.
* Passing raw messages to `BinanceParser` for message identification and parsing.
* Passing order-book updates to `OrderBookReconstructor`.
* Detecting synchronization failures.
* Triggering snapshot recovery when a sequence gap is detected.

`BinanceClient` does not own generic HTTP communication.

REST operations are delegated to dedicated Binance components using the shared `HttpClient`.

The client therefore acts primarily as an orchestration layer between Binance's streaming API and FinPulse's market-data components.

### **Market Data Routing**

`BinanceClient` receives messages from Binance's combined WebSocket stream and routes them according to their message type.

Trade messages are identified and parsed by `BinanceParser` into canonical `Trade` objects. The resulting trades are passed to `CandleAggregator` for candle construction.

Order-book update messages are identified and parsed by `BinanceParser` into `BinanceOrderBookUpdate` objects. These updates are passed to `OrderBookReconstructor` for state reconstruction.

This allows BinanceClient to act as an orchestration layer while keeping candle aggregation independent of Binance-specific logic. Order-book reconstruction remains within the Binance-specific ingestion layer because it depends on Binance-specific snapshot, update, and sequencing models.

**Current routing:**

`Binance WebSocket → BinanceParser → Trade → CandleAggregator → CandleStick`

`Binance WebSocket → BinanceParser → BinanceOrderBookUpdate → OrderBookReconstructor → OrderBook`

---

## **BinanceOrderBookSnapshotRetriever**

`BinanceOrderBookSnapshotRetriever` is responsible specifically for retrieving Binance order-book snapshots.

It:

1. Constructs the Binance depth endpoint.
2. Uses `HttpClient` to perform the HTTPS request.
3. Passes the returned JSON to `BinanceParser`.
4. Returns a `BinanceOrderBookSnapshot`.

It does not implement HTTP connection logic itself.

---

# **Parsing Layer**

## **BinanceParser**

`BinanceParser` converts Binance-specific JSON messages into structured models.

It currently handles:

* Binance trade messages.
* Binance incremental order-book updates.
* Binance order-book snapshots.

The parser is responsible for interpreting Binance's JSON schema but is not responsible for networking or maintaining market state.

This creates a clean boundary between external payloads and internal application logic.

---

# **Order Book State Management**

## **OrderBookReconstructor**

`OrderBookReconstructor` maintains the current state of a canonical `OrderBook`.

It does not communicate with Binance directly.

Its responsibilities are:

* Initializing the book from a snapshot.
* Applying incremental updates.
* Ignoring updates that have already been processed.
* Detecting update-ID gaps.
* Removing price levels whose quantity becomes zero.
* Updating existing price levels.
* Adding new price levels.
* Producing the current canonical `OrderBook`.

### **Initialization**

A REST snapshot establishes the initial state of the order book.

The snapshot's `last_update_id` becomes the starting sequence position of the reconstructed book.

### **Incremental Updates**

After initialization, Binance WebSocket updates are applied incrementally.

Each update contains:

* First update ID.
* Final update ID.
* Bid changes.
* Ask changes.
* Symbol.
* Event timestamp.

The reconstructor verifies that the update sequence is compatible with the current state.

Updates are classified as:

* `Applied`
* `Ignored`
* `SequenceGap`

`Ignored` updates are older than or already covered by the current state.

`Applied` updates safely modify the current book.

`SequenceGap` indicates that the current reconstructed state can no longer be trusted and requires recovery.

---

# **Order Book Recovery**

When `OrderBookReconstructor` reports a `SequenceGap`, the Binance client requests a new REST snapshot.

The recovery process:

1. Detect the sequence gap.
2. Request a fresh Binance order-book snapshot.
3. Parse the snapshot.
4. Reinitialize `OrderBookReconstructor`.
5. Resume processing WebSocket updates.

This allows the live order book to recover from missed or invalidated update sequences without restarting the entire client.

The recovery mechanism has been tested against live Binance market data. The reconstructed order book successfully initialized from a REST snapshot and continued applying subsequent WebSocket depth updates.

---

# **Analysis**

FinPulse separates market data from derived analysis.

Canonical domain models represent facts about the market and remain independent of analytical calculations.

Analysis modules consume these models and produce derived metrics without modifying the underlying market data.

For example, candle-related analysis is separated into:

* `CandleMetrics`: structural properties of a candle.
* `ATR`: a stateful streaming volatility calculation.
* `VolatilityMetrics`: volatility measurements derived from candle metrics and ATR.

This separation allows analytical modules to evolve independently from the canonical market-data modules and makes it possible to add new indicators or machine-learning features without changing the underlying market-data representation.

---

# **Streaming Analysis**

FinPulse performs certain analytical calculations incrementally as market data arrives rather than repeatedly recalculating them from historical data.

ATR is implemented as a stateful streaming calculation.

After initialization, each new candle requires only:

* The previous ATR.
* The new True Range.
* The previous candle's close.

This allows ATR to be updated in constant time per candle while avoiding unnecessary historical scans.

---

# **Data Flow**

FinPulse currently supports two primary paths for Binance order-book data.

### **REST Snapshot**

The REST path obtains an initial or recovery snapshot of the order book.

`HttpClient` handles generic HTTPS communication, while Binance-specific components handle endpoint construction and response parsing.

### **Live WebSocket**

The WebSocket path receives incremental order-book updates.

`BinanceClient` manages the connection and coordinates the parser and order-book reconstructor.

### **Synchronization**

The REST snapshot and WebSocket stream work together to maintain a synchronized order book.

A sequence gap invalidates the current state and triggers snapshot recovery.

> **Architecture diagrams will be added later once the backend architecture and storage pipeline are finalized.**

---

# Storage and Buffering

**NOTE: storage and buffering is in need of stress testing still!**

## Storage

The `Storage` component provides the interface between market-data ingestion and persistent database storage.

It owns separate buffers for each persistable domain model:

* `Trade`
* `CandleStick`

`Storage` delegates database operations to the appropriate buffer, keeping database-specific logic separate from the ingestion layer.

The `Database` component is responsible for executing the actual TimescaleDB operations. It provides insertion functions for supported domain models and reports the result through `DatabaseResults`.

## Buffer

`Buffer<T>` is a generic templated component responsible for temporarily holding data before it is persisted.

Each buffer has:

* A primary buffer with a fixed capacity.
* An overflow buffer with a separate capacity.
* A mutex protecting buffer state.
* A reference to the `Database` component.

The buffer reports its current submission state through `BufferResult`:

* `Buffer` — data was accepted by the primary buffer.
* `Overflow` — data was accepted by the overflow buffer or the primary buffer is being refilled from overflow.
* `Full` — both available buffer capacities have been exhausted.

The buffer also handles moving data between the overflow and primary buffers as capacity becomes available.

## Database Failure Handling

Database insertion failures are represented through `DatabaseResults`.

When a broken database connection is detected, the `Database` component attempts to reconnect and reports the resulting connection status to the caller.

The buffering system is designed so that temporary database unavailability can be handled without stopping market-data ingestion. Data can remain buffered while database capacity is unavailable, subject to the configured buffer capacities.

## Synchronization

Buffer operations are protected by a mutex.

`consume()` is the synchronized public operation, while `consume_unlocked()` is used internally when the caller already holds the buffer mutex. This prevents attempting to acquire the same mutex recursively during internal buffer operations.

## Back Pressure
**NOT IMPLEMENTED YET!**

The buffering system provides bounded storage through its primary and overflow capacities.

When both buffers are full, `BufferResult::Full` is returned to the caller. The ingestion layer can use this result to stop submitting additional data to storage without stopping unrelated market-data processing.

The buffering design is intentionally compatible with the planned asynchronous migration. The storage subsystem can later be moved to asynchronous consumption without requiring the market-data models or storage interface to be redesigned.

**Note: will explain how the buffer is being implemented soon!**
---

# **Project Structure**

The backend is organized into separate `include/` and `src/` trees.

`include/` contains header files and defines the interfaces, models, and class declarations used throughout the backend.

`src/` contains implementations for components that require a corresponding source file. Not every header has an implementation file; some headers contain definitions that do not require a separate `.cpp` file.

The backend also contains a `build/` directory generated by CMake and used for compiled build artifacts.

## **backend/include**

### **`aggregation/`**
Contains analysis components that operate on canonical market-data models.

* `candle_aggregator.hpp`

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
* `database_results.hpp`
* `storage.hpp`

The storage layer is currently under development.

### **`storage/buffer/`**

contains buffering abstraction and template.

* `buffer_results.hpp`
* `buffer.hpp`
* `database.tpp`

---

## **backend/src**

The `src/` directory contains implementations for backend components that require separate compilation units.

### **`aggregation/`**
Contains analysis components that operate on canonical market-data models.

* `candle_aggregator.cpp`

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

# **Design Decisions**

## **Canonical Market Data**

All exchanges can represent the same market information differently.

FinPulse converts exchange-specific representations into canonical domain models so downstream systems remain exchange-independent.

## **MarketIngestor Interface**

`MarketIngestor` defines the common interface that exchange-specific ingestion clients are expected to implement.

This allows new exchanges to be added without requiring changes to storage, visualization, replay, or analysis modules.

## **Exchange Isolation**

Exchange-specific networking, API endpoints, JSON schemas, and synchronization rules remain inside exchange-specific ingestion components.

Reason being is because trying to make one generic component for all exchanges will lead to a fruitless endeavour, where it'll create a cluttered and too far abstracted work environment.

Downstream systems operate on canonical models rather than exchange payloads. This simplifies the sharing of data between components and infrastructure.

## **Generic Transport Infrastructure**

Generic networking functionality belongs in reusable infrastructure such as `HttpClient`.

Exchange components should configure and consume this infrastructure rather than reimplementing HTTPS/TLS connection boilerplate.

## **Single Responsibility**

As established in the philosophy section, components should perform one primary responsibility.

For example:

* `HttpClient` handles HTTP transport.
* `BinanceOrderBookSnapshotRetriever` handles Binance snapshot retrieval.
* `BinanceParser` handles Binance payload parsing.
* `OrderBookReconstructor` handles order-book state.
* `BinanceClient` coordinates the live Binance ingestion process.

This prevents exchange clients from becoming large collections of unrelated networking, parsing, state-management, and persistence logic.

## **Boost.Beast**

Boost.Beast was selected for WebSocket communication due to its native integration with Boost.Asio and its mature implementation.

## **Boost.Asio**

Boost.Asio provides the underlying networking primitives used by FinPulse's HTTP and WebSocket infrastructure.

The current Binance WebSocket implementation is synchronous, with asynchronous networking planned for future development.

## **Conan**

Conan was selected as the package manager for FinPulse because it provides reproducible dependency management, consistent library versions across development environments, and straightforward integration with CMake.

## **Storage**
will document later.
