#pragma once

#include <map>

#include "models/order_book/order_book.hpp"
#include "ingestion/binance/updates/binance_order_book_updates.hpp"
#include "ingestion/binance/updates/binance_order_book_snapshot.hpp"

class OrderBookReconstructor
{
private:
    std::map<double, double> bids;
    std::map<double, double> asks;

    std::string symbol;

    std::chrono::system_clock::time_point timestamp;

public:
    // initialize snapshit of first order book
    void initialize(const BinanceOrderBookSnapshot& snapshot);

    void apply_update(const BinanceOrderBookUpdate& update);

    OrderBook get_order_book() const;
};