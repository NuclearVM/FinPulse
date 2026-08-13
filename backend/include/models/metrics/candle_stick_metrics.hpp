#pragma once

#include "models/candle_stick.hpp"

struct CandleMetrics
{
    double range;
    double body;
    double upper_wick;
    double lower_wick;
};

// IMPORTANT: NEED to create candles from trades later
// collect data from trades to create the initial candles
// new component to add later: candle agregator or something along those lines