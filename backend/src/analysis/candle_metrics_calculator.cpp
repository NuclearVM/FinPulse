#include <algorithm>
#include <cmath>
#include "analysis/candle_metrics_calculator.hpp"

CandleMetrics CandleMetricsCalculator::calculate(const CandleStick& candle) const {

    CandleMetrics metrics;

    metrics.range = calculate_range(candle);
    metrics.body = calculate_body(candle);
    metrics.upper_wick = calculate_upper_wick(candle);
    metrics.lower_wick = calculate_lower_wick(candle);

    return metrics;
}

double CandleMetricsCalculator::calculate_range(const CandleStick& candle) const {return candle.high - candle.low; }

double CandleMetricsCalculator::calculate_body(const CandleStick& candle) const {return std::abs(candle.close - candle.open); }

double CandleMetricsCalculator::calculate_upper_wick(const CandleStick& candle) const {return candle.high - std::max(candle.open, candle.close); }

double CandleMetricsCalculator::calculate_lower_wick(const CandleStick& candle) const {return std::min(candle.open, candle.close) - candle.low; }