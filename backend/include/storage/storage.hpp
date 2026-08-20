#include "models/candle_stick.hpp"
#include "models/trade.hpp"
#include "database.hpp"
#include "buffer/buffer.hpp"

class Storage
{
private:
    Database database;

    Buffer<Trade> trade_buffer;
    Buffer<CandleStick> candle_buffer;

public:
    explicit Storage(const DatabaseConfig& config);
};