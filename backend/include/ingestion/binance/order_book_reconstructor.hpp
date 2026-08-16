#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <stdexcept>
#include "models/order_book/order_book.hpp"
#include "ingestion/binance/updates/binance_order_book_updates.hpp"
#include "ingestion/binance/updates/binance_order_book_snapshot.hpp"
#include "ingestion/binance/updates/update_results.hpp"

class OrderBookReconstructor
{
private:
    std::map<double, double> bids;
    std::map<double, double> asks;

    std::string symbol;
    std::string exchange;

    std::uint64_t last_update_id = 0;

    std::chrono::system_clock::time_point timestamp;

public:
    // initialize snapshot of first order book
    void initialize(const BinanceOrderBookSnapshot& snapshot);

    UpdateResult apply_update(const BinanceOrderBookUpdate& update);

    OrderBook get_order_book() const;
};