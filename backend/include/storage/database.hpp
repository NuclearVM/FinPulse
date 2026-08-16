#pragma once

#include "models/trade.hpp"
#include "models/candle_stick.hpp"
#include "common/config.hpp"

#include <pqxx/pqxx>

class Database {
private:
    pqxx::connection connection;

public:
    explicit Database(const DatabaseConfig& config);

    void insert_trades(const Trade& trade);
    void insert_candles(const CandleStick& candle);
};