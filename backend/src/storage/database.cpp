#include "storage/database.hpp"

#include <stdexcept>
#include <iostream>
#include <chrono>

// remove test prints later

Database::Database(const DatabaseConfig& config) : 
                                connection(
                                    "host=" + config.host +
                                    " port=" + config.port +
                                    " dbname=" + config.database +
                                    " user=" + config.user +
                                    " password=" + config.password
                                ) {
    if (!connection.is_open())
    {
        throw std::runtime_error("Failed to open database connection");
    }

    std::cout << "Connected to database\n";
}

DatabaseResults Database::insert_trades(const Trade& trade) 
{
    try 
    {
        pqxx::work transaction(connection);

        // converts trade timestamps to epoch microseconds
        const auto event_time_us = std::chrono::duration_cast<std::chrono::microseconds>(trade.event_time.time_since_epoch()).count();

        const auto execution_time_us = std::chrono::duration_cast<std::chrono::microseconds>(trade.execution_time.time_since_epoch()).count();

        const auto received_time_us = std::chrono::duration_cast<std::chrono::microseconds>(trade.received_time.time_since_epoch()).count();

        transaction.exec(
            R"(
                INSERT INTO trades (
                    trade_id,
                    exchange,
                    symbol,
                    price,
                    quantity,
                    buyer_is_maker,
                    event_time,
                    execution_time,
                    received_time
                )
                VALUES (
                    $1,
                    $2,
                    $3,
                    $4,
                    $5,
                    $6,
                    to_timestamp($7 / 1000000.0),
                    to_timestamp($8 / 1000000.0),
                    to_timestamp($9 / 1000000.0)
                )
            )",
            pqxx::params{
            trade.trade_id,
            trade.exchange,
            trade.symbol,
            trade.price,
            trade.quantity,
            trade.buyer_is_maker,
            event_time_us,
            execution_time_us,
            received_time_us
            }
        );

    transaction.commit();

    }
    catch (const pqxx::broken_connection&) 
    {
        return DatabaseResults::CONNECTION_ERROR;
    }
    catch (const std::exception&) 
    {
        return DatabaseResults::QUERY_ERROR;
    }
}

DatabaseResults Database::insert_candles(const CandleStick& candle)
{
    try
    {
        pqxx::work transaction(connection);

        // converts candle timestamp to epoch microseconds
        const auto timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(candle.timestamp.time_since_epoch()).count();

        transaction.exec(
            R"(
                INSERT INTO candles (
                    exchange,
                    symbol,
                    candle_time,
                    open_price,
                    high,
                    low,
                    close_price,
                    volume
                )
                VALUES (
                    $1,
                    $2,
                    to_timestamp($3 / 1000000.0),
                    $4,
                    $5,
                    $6,
                    $7,
                    $8
                )
            )",
            pqxx::params{
            candle.exchange,
            candle.symbol,
            timestamp_us,
            candle.open,
            candle.high,
            candle.low,
            candle.close,
            candle.volume
            }
        );

        transaction.commit();
    }
    catch (const pqxx::broken_connection&) 
    {
        return DatabaseResults::CONNECTION_ERROR;
    }
    catch (const std::exception&) 
    {
        return DatabaseResults::QUERY_ERROR;
    }
    
