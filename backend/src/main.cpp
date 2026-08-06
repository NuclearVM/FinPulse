#include <iostream>
#include <iomanip>
#include "ingestion/binance/binance_client.hpp"
#include "models/trade.hpp"

int main() {
    // BinanceClient client;
    // std::cout << "Client created\n";

    // client.connect();

    // Trade trade;

    BinanceClient client;

    //  fake test trade
    client.set_trade_callback(
        [](const Trade& trade)
        {
            std::cout
                << trade.symbol
                << " "
                << std::fixed
                << std::setprecision(8)
                << trade.price
                << " "
                << trade.quantity
                << '\n';
        }
    );

    client.connect();

    client.start();

    return 0;
}