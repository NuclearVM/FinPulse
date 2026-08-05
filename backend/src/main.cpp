#include <binance_client.hpp>
#include <iostream>

int main() {
    BinanceClient client;
    std::cout << "Client created\n";

    client.connect();
}