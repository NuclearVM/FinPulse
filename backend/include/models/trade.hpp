#pragma once

#include <string>
#include <chrono>

struct Trade
{
    std::string source;
    std::string symbol;

    double price;
    double quantity;

    std::chrono::system_clock::time_point timestamp;

    // need to make two separate time stamps later
    // one for when binance made the exchange and
    // when finpulse receives it

    // who provided liquidity
    bool buyer_is_maker;
};