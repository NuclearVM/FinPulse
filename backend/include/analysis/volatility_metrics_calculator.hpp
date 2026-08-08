#pragma once

#include "models/metrics/volatility_metrics.hpp"
#include "models/metrics/candle_stick_metrics.hpp"

class VolatilityMetricsCalculator {

public:
    VolatilityMetrics calculate(const CandleMetrics& candle_metrics, double atr) const;
};