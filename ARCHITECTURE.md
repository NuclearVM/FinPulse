# **Overview**
FinPulse is a modular market data platform designed to ingest, store, analyze, and visualize financial market data. The backend is organized around stable abstractions that allow new exchanges, storage backends, and analysis modules to be added with minimal changes to existing code.

# **Philosophy**
FinPulse is designed around a few core principles:

- Every component should have a single, well-defined responsibility.

- Components should be loosely coupled and communicate through stable interfaces.

- Exchange-specific logic should remain isolated from the rest of the system.

- Adding new functionality should require extending the system rather 
than modifying existing components.

- Domain models should provide a common representation of market data regardless of its source.

The long-term goal is to make FinPulse easy to extend. If a new exchange, database, visualization, or machine learning module is added months later, the existing architecture should require little to no modification.

# **Design Decisions**
### Canonical Trade Model

All exchanges pass different JSON formats. FinPulse converts these payloads into a common Trade model so downstream systems remain exchange-independent. 

### MarketIngestor Interface

All market data providers implement a common interface.

This allows new exchanges to be added without modifying storage, visualization, replay, or analysis modules.

### Canonical Market Data and Analysis

FinPulse separates market data from derived analysis.

Canonical domain models represent facts about the market and remain independent of analytical calculations (more in Canonical Trade Model section in backend). Examples include `Trade`, `CandleStick`, and `OrderBook`.

Analysis modules consume these models and produce derived metrics without modifying the underlying market data.

For example, candle analysis is separated into:

- `CandleMetrics`: structural properties of a candle.

- `ATR`: a stateful streaming volatility calculation.

- `VolatilityMetrics`: volatility measurements derived from candle metrics and ATR.

This separation allows analytical modules to evolve independently from the canonical market data modules and makes it possible to add new indicators or machine learning features without changing the underlying market-data representation.

### Streaming Analysis

FinPulse performs certain analytical calculations incrementally as market data arrives rather than repeatedly recalculating them from historical data.

ATR is implemented as a stateful streaming calculation. After initialization, each new candle requires only the previous ATR, the new True Range, and the previous candle's close.

This allows ATR to be updated in constant time per candle while avoiding unnecessary historical scans.

### Boost.Beast

Boost.Beast was selected for WebSocket communication due to its native integration with Boost.Asio and its mature, production-ready implementation.

### Conan

Conan was selected as the package manager for FinPulse because it provides reproducible dependency management, consistent library versions across development environments, and straightforward integration with CMake.

# **Project Structure**
### backend

**ingestion:** Responsible for communicating with external data providers and parsing. Contains the MarketIngestor interface and exchange-specific clients.

**models:** Contains the canonical domain models used throughout FinPulse. Every subsystem communicates using these models rather than exchange-specific payloads.

**analysis:** Contains calculations and derived metrics built from canonical market-data models. Analaysis modules do not own or modify the un derlying market data.

**storage:** Responsible for persistence, caching, and database communication.

**common:** Shared utilities used across multiple modules. This directory is reserved for reusable infrastructure and should never become a miscellaneous dumping ground.

