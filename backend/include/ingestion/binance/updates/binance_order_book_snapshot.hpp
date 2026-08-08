#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "models/order_book/order_book_level.hpp"

struct BinanceOrderBookSnapshot
{
    std::string symbol;

    std::uint64_t last_update_id;

    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;
};