#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "analysis/volatility/atr.hpp"

ATR::ATR(std::size_t period) : period(period) {

    if (period == 0)
    {
        throw std::invalid_argument("ATR period must be greater than zero");
    }
    
}

double ATR::true_range(const CandleStick& current, const CandleStick& previous) const {

    const double high_low = current.high - current.low;

    const double high_previous_close = std::abs(current.high - previous.close);

    const double low_previous_close = std::abs(current.low - previous.close);

    return std::max({high_low, high_previous_close, low_previous_close});
}

std::optional<double> ATR::update(const CandleStick& candle) {

    if (!previous_candle) {

        previous_candle = candle;
        
        return std::nullopt;
    }

    const double tr = true_range(candle, *previous_candle);

    ++count; 

    if (count < period) {current_atr += tr; }

    else if (count == period) {current_atr = (current_atr + tr) / period; }

    else {current_atr = ((current_atr * (period - 1)) + tr) / period; }

    previous_candle = candle;

    if (count < period) return std::nullopt;

    return current_atr;
}