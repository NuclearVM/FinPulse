#pragma once

#include "models/trade.hpp"
#include "models/candle_stick.hpp"
#include "common/config.hpp"
#include "database_results.hpp"

#include <pqxx/pqxx>
#include <chrono>
#include <string>

class Database {
private:
    std::string connection_string;
    pqxx::connection connection;

    DatabaseResults reconnect();

public:
    explicit Database(const DatabaseConfig& config);

    DatabaseResults insert_trades(const Trade& trade);
    DatabaseResults insert_candles(const CandleStick& candle);
};