#pragma once

#include <chrono>
#include <string>
#include <vector>
#include "models/order_book/order_book_level.hpp"

struct OrderBook {
    
    std::string symbol;
    std::string exchange;

    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;

    std::chrono::system_clock::time_point timestamp;
};