#pragma once

#include "models/trade.hpp"

#include <string>


class BinanceParser {

private:

    // use for conversion
    std::chrono::system_clock::time_point convert_timestamp(uint64_t timestamp);

public:

    Trade parse_trade(const std::string& message);

};