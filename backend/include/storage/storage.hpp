#pragma once

#include "models/candle_stick.hpp"
#include "models/trade.hpp"
#include "database.hpp"
#include "buffer/buffer.hpp"

class Storage
{
private:
    Database database;
    DatabaseResults db_result;

    Buffer<Trade> trade_buffer;
    Buffer<CandleStick> candle_buffer;

public:
    explicit Storage(const DatabaseConfig& config);

    BufferResult submit(const Trade& trade);
    BufferResult submit(const CandleStick& candle);

    DatabaseResults get_db_result() const;

    void drain();
};