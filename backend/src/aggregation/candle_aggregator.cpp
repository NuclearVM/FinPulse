#include "aggregation/candle_aggregator.hpp"

CandleAggregator::CandleAggregator(std::chrono::seconds timeframe) : timeframe(timeframe) {}

std::optional<CandleStick> CandleAggregator::process_trade(const Trade& trade)
{   
    // initial candle, sets everything to the values of one trade
    if (!has_current_candle)
    {
        current_candle.symbol = trade.symbol;
        current_candle.exchange = trade.exchange;
        current_candle.open = trade.price;
        current_candle.close = trade.price;
        current_candle.high = trade.price;
        current_candle.low = trade.price;
        current_candle.volume = trade.quantity;
        current_candle.timestamp = trade.event_time;

        has_current_candle = true;

        return std::nullopt;
    }

    const auto elapsed = trade.event_time - current_candle.timestamp;

    if (elapsed < timeframe)
    {
        current_candle.close = trade.price;
        current_candle.high = std::max(current_candle.high, trade.price);
        current_candle.low = std::min(current_candle.low, trade.price);
        current_candle.volume += trade.quantity;

        return std::nullopt;
    }

    CandleStick completed_candle = current_candle;

    current_candle.symbol = trade.symbol;
    current_candle.exchange = trade.exchange;
    current_candle.open = trade.price;
    current_candle.close = trade.price;
    current_candle.high = trade.price;
    current_candle.low = trade.price;
    current_candle.volume = trade.quantity;
    current_candle.timestamp = trade.event_time;

    return completed_candle;
}