# FinPulse
FinPulse is a real-time financial market data platform that ingests live market data, stores historical price activity, caches frequently accessed information for low-latency retrieval, and exposes financial data through APIs and visualization tools. 

## Project Age

This project is **3 weeks old.** It is actively being built, and the core backend is already functional.

The repository is temporarily open to provide recruiters and interviewers with visibility into my engineering process, architecture, and code quality. It is not yet intended for local execution.

---

## What's Been Built in 3 Weeks

- Binance WebSocket ingestion (Boost.Beast / Boost.Asio)
- Order book reconstruction with sequence gap recovery
- ATR streaming (Wilder's smoothing, O(1))
- Candle aggregation from trades
- CandleMetrics + VolatilityMetrics
- TimescaleDB integration
- Generic buffering and storage patterns
- Modern C++ (RAII, exceptions, optionals)
- Modular architecture with exchange isolation
- A bunch of boiler plate code

## If you still want to run it

I don't recommend running it yet as it only pushes data into timescaleDB at the moment, however if you still want to run it:

- clone the repo
- open it in your code editor
- run: `docker compose up -d` in your terminal
- then: `cd backend`
- and finally: `./run.sh`

# To shut down the container

Run `docker compose down` or `docker compose down -v` if you want to get rid of all stored data.

Again, bear with me as this project is still under active development.

**Next up:** Redis caching, Electron frontend, multi-threading, switching to asynchronous data retrieval, and the ML module.