#include "storage/storage.hpp"

Storage::Storage(const DatabaseConfig& config)
                                : database(config),
                                trade_buffer(database, 10000, 50000),
                                candle_buffer(database, 7000, 30000) {}

BufferResult Storage::submit(const Trade& trade)
{
    // temporary
    auto result = trade_buffer.submit(trade);
    db_result = trade_buffer.get_db_result();

    return result;
}

BufferResult Storage::submit(const CandleStick& candle)
{
    auto result = candle_buffer.submit(candle);
    db_result = candle_buffer.get_db_result();

    return result;
}

void Storage::drain() 
{
    trade_buffer.drain();
    candle_buffer.drain();
}

DatabaseResults Storage::get_db_result() const
{
    return db_result;
}

