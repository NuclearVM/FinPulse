#pragma once
#include <string>
#include <chrono>
#include <cstdint>
#include <optional>
#include "models/candle_stick.hpp"
#include "models/trade.hpp"

// if there's any build probelms check cmake, you fucked around by moving this directory a lot
class CandleAggregator
{
private:
    std::chrono::seconds timeframe;

    CandleStick current_candle;
    bool has_current_candle = false;

public:
    explicit CandleAggregator(std::chrono::seconds timeframe);
    // processes trades by creating and updating candles.
    // Returns a completed candle when the timeframe rolls over.
    std::optional<CandleStick> process_trade(const Trade& trade);

};