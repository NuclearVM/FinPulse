#include "ingestion/binance/binance_client.hpp"
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

        connected = true;

        std::cout << "connected to binance" << '\n';

    }
    catch(const std::exception& e) {

        connected = false;

        std::cerr << "Connection failed: " << e.what() << '\n';
    }
    
}

void BinanceClient::disconnect() {

    try {
        
        websocket.close(boost::beast::websocket::close_code::normal);

         connected = false;

        std::cout << "Disconnected from Binance" << '\n';

    }
    catch(const std::exception& e) {

        connected = false;

        std::cerr << "Disconnect error: " << e.what() << '\n';
    }
    
}

std::string BinanceClient::read_message() {

    websocket.read(buffer);

    // std::string message(
    //     static_cast<const char*>(buffer.data().data()),
    //     buffer.size()
    // );

    auto message = boost::beast::buffers_to_string(buffer.data());

    buffer.consume(buffer.size());

    return message;
}

void BinanceClient::start() {

    while (connected) {

        try {
            auto message = read_message();

            Trade trade = parser.parse_trade(message);

            if (trade_callback) {
                trade_callback(trade);
            }
        }
        catch (const std::exception& e) {

            std::cerr << "Market data error: " << e.what() << '\n';
        }

        // std::cout << message << '\n';

    }
}