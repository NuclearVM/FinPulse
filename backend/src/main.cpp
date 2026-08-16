#include <iostream>
#include "ingestion/binance/binance_client.hpp"
#include "storage/database.hpp"
#include "common/config.hpp"

int main() {

    // BinanceClient client;

    // client.connect();
    // client.start();

    DatabaseConfig config = load_database_config();

    Database database(config);


    return 0;
}