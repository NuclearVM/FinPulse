#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include "models/order_book/order_book_level.hpp"

struct BinanceOrderBookUpdate
{
    std::string symbol;

    std::uint64_t first_update_id;
    std::uint64_t final_update_id;

    std::chrono::system_clock::time_point timestamp;

    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;
};