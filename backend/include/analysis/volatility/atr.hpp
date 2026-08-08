#pragma once

#include <cstddef>
#include <optional>
#include "models/candle_stick.hpp"

// ATR stands for average true range

class ATR {

    private:
        std::size_t period;
        std::size_t count = 0;

        // change to more accurate name later
        double current_atr = 0.0;

        std::optional<CandleStick> previous_candle;

        double true_range(const CandleStick& current, const CandleStick& previous) const;

    public:
        explicit ATR(std::size_t period);

        std::optional<double> update(const CandleStick& candle);
};