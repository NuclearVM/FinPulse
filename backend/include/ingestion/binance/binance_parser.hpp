#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include "models/trade.hpp"
#include "ingestion/binance/updates/binance_order_book_updates.hpp"
#include "ingestion/binance/updates/binance_order_book_snapshot.hpp"


class BinanceParser {

private:

    // use for conversion
    std::chrono::system_clock::time_point convert_timestamp(uint64_t timestamp);

public:

    Trade parse_trade(const std::string& message);

    BinanceOrderBookUpdate parse_order_book_updates(const std::string& message);

    BinanceOrderBookSnapshot parse_order_book_snapshot(const std::string& message);

};