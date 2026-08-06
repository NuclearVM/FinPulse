#include "ingestion/binance_client.hpp"
#include <iostream>


// need to change to asynch later
BinanceClient::BinanceClient() : ssl_context(boost::asio::ssl::context::tls_client), resolver(io_context), websocket(io_context, ssl_context) {

}

void BinanceClient::connect() {
    try
    {
        // get host
        auto endpoints = resolver.resolve("stream.binance.com", "9443");
        
        // tcp connecting
        boost::asio::connect(websocket.next_layer().next_layer(), endpoints);

        // tls handshake
        websocket.next_layer().handshake(boost::asio::ssl::stream_base::client);

        // websocket handshake
        websocket.handshake("stream.binance.com", "/ws/btcusdt@trade");

        std::cout << "connected to binance" << '\n';

    }
    catch(const std::exception& e)
    {
        std::cerr << "Connection failed: " << e.what() << '\n';
    }
    
}

void BinanceClient::disconnect() {

    try
    {
        websocket.close(boost::beast::websocket::close_code::normal);

        std::cout << "Disconnected from Binance" << '\n';

    }
    catch(const std::exception& e)
    {
        std::cerr << "Disconnect error: " << e.what() << '\n';
    }
    
}

void BinanceClient::start() {
    // will become the receive loop later
}