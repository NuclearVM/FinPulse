#include <iostream>
#include "ingestion/binance/binance_client.hpp"
#include "storage/database.hpp"
#include "common/config.hpp"

int main() 
{
    DatabaseConfig config = load_database_config();

    BinanceClient client(config);

    client.connect();
    client.start();

    return 0;
}