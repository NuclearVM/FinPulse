#pragma once

#include<functional>
#include<models/trade.hpp>

class MarketIngestor {

protected:
    std::function<void(const Trade&)> trade_callback;

public:

    virtual void connect() = 0;

    virtual void disconnect() = 0;

    virtual void start() = 0;

    void set_trade_callback(std::function<void(const Trade&)> callback) {
        trade_callback = callback;
    }

    virtual ~MarketIngestor() = default;

};