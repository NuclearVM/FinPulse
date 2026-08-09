#include <iostream>
#include "ingestion/binance/binance_client.hpp"


int main() {

    BinanceClient client;

    client.connect();
    client.start();

    return 0;
}
