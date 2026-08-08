#include "analysis/volatility_metrics_calculator.hpp"

VolatilityMetrics VolatilityMetricsCalculator::calculate(const CandleMetrics& candle_metrics, double atr) const {

    VolatilityMetrics metrics{};

    metrics.atr = atr;

    metrics.range_atr = candle_metrics.range / atr;

    metrics.body_atr = candle_metrics.body / atr;

    return metrics;

}