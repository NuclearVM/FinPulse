#include "ingestion/binance/binance_client.hpp"
#include <iostream>


// need to change to asynch later DO NOT FORGET
BinanceClient::BinanceClient() : ssl_context(boost::asio::ssl::context::tls_client), 
                                resolver(io_context), 
                                websocket(io_context, ssl_context), 
                                http_client(io_context, ssl_context), 
                                snapshot_retriever(http_client, parser) {}

void BinanceClient::connect() {
    try
    {  

        // get host
        auto endpoints = resolver.resolve("stream.binance.com", "9443");
        
        // tcp connecting
        boost::asio::connect(websocket.next_layer().next_layer(), endpoints);

        // tls handshake
        websocket.next_layer().handshake(boost::asio::ssl::stream_base::client);

        // websocket handshake, gets trades
        // websocket.handshake("stream.binance.com", "/ws/btcusdt@trade");

        // get order book
        websocket.handshake("stream.binance.com", "/ws/btcusdt@depth");

        connected = true;

        std::cout << "connected to binance" << '\n';

    }
    catch(const std::exception& e) {

        connected = false;

        std::cerr << "Connection failed: " << e.what() << '\n';
    }
    
}

void BinanceClient::disconnect() {

    if (!connected) return; 

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

    const auto message = boost::beast::buffers_to_string(buffer.data());

    buffer.consume(buffer.size());

    return message;
}

void BinanceClient::initialize_order_book() {

        auto snapshot = snapshot_retriever.get_snapshot("BTCUSDT", 100);

        order_book_reconstructor.initialize(snapshot);

        std::cout << "Order book initialized at update ID " << snapshot.last_update_id << '\n';

        
}

void BinanceClient::recover_order_book() {

    std::cerr << "Order book sequence gap detected. " << "Recovering...\n";

    initialize_order_book();

    std::cout << "Order book recovered.\n";
}

void BinanceClient::start() {

     if (!connected)
    {
        throw std::runtime_error(
            "Cannot start Binance client: not connected"
        );
    }

    while (connected) {
        try {
            
            auto message = read_message();

            auto update = parser.parse_order_book_updates(message);

            auto result = order_book_reconstructor.apply_update(update);

            switch (result) {

                case UpdateResult::Applied:
                    break;

                case UpdateResult::Ignored:
                    break;

                case UpdateResult::SequenceGap:
                    recover_order_book();
                    break;
            }
        }

        catch (const std::exception& e) {

            std::cerr << "Market data error: " << e.what() << '\n';
        }
    }
}