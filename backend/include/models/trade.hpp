#pragma once

#include <string>
#include <chrono>
#include <cstdint>

struct Trade
{
    std::string exchange;
    std::string symbol;

    double price;
    double quantity;

    std::uint64_t trade_id;

    std::chrono::system_clock::time_point event_time;
    std::chrono::system_clock::time_point execution_time;
    std::chrono::system_clock::time_point received_time;

    // who provided liquidity
    bool buyer_is_maker;
};