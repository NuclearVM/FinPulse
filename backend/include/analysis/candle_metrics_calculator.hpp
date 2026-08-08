#pragma once

#include "models/candle_stick.hpp"
#include "models/metrics/candle_stick_metrics.hpp"

class CandleMetricsCalculator {
private:
    double calculate_range(const CandleStick& candle) const;

    double calculate_body(const CandleStick& candle) const;

    double calculate_upper_wick(const CandleStick& candle) const;

    double calculate_lower_wick(const CandleStick& candle) const;

public:
    CandleMetrics calculate(const CandleStick& candle) const;

};