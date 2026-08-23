#include "ingestion/binance/binance_client.hpp"
#include <iostream>


// need to change to asynch later DO NOT FORGET
BinanceClient::BinanceClient(const DatabaseConfig& config) : ssl_context(boost::asio::ssl::context::tls_client), 
                                resolver(io_context), 
                                websocket(io_context, ssl_context), 
                                http_client(io_context, ssl_context), 
                                snapshot_retriever(http_client, parser),
                                candle_aggregator(std::chrono::seconds(60)),
                                storage(config) {}

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
        // websocket.handshake("stream.binance.com", "/ws/btcusdt@depth");
        
        // get both trades and order book
        websocket.handshake("stream.binance.com", "/stream?streams=btcusdt@trade/btcusdt@depth");
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

    std::size_t applied_updates = 0;

    if (!connected)
    {
        throw std::runtime_error(
            "Cannot start Binance client: not connected"
        );
    }

    while (connected)
    {
        try
        {
            auto message = read_message();

            switch (parser.identify_message(message))
            {
                case BinanceMessageType::Trade:
                {
                    auto trade = parser.parse_trade(message);
                    
                    // std::cout<< "TRADE: "
                    // << trade.symbol
                    // << " price=" << trade.price
                    // << " quantity=" << trade.quantity
                    // << '\n';

                    trade_result = storage.submit(trade);

                    switch (trade_result) 
                    {
                        case BufferResult::Buffer:
                            // std::cout << "TRADE -> BUFFER\n";
                            break;

                        case BufferResult::Overflow:
                            // std::cout << "TRADE -> OVERFLOW\n";
                            break;

                        case BufferResult::Full:
                            // std::cout << "TRADE -> FULL\n";
                            // have the client stop submitting trades and candles
                            break;
                    }

                    auto completed_candle = candle_aggregator.process_trade(trade);

                    if (completed_candle)
                    {
                        //                          std::cout
                        // << "CANDLE COMPLETED: "
                        // << completed_candle->symbol
                        // << " O=" << completed_candle->open
                        // << " H=" << completed_candle->high
                        // << " L=" << completed_candle->low
                        // << " C=" << completed_candle->close
                        // << " V=" << completed_candle->volume
                        // << '\n';

                        candle_result = storage.submit(*completed_candle);
                    }

                    switch (candle_result) 
                    {
                        case BufferResult::Buffer:
                            // std::cout << "TRADE -> BUFFER\n";
                            break;

                        case BufferResult::Overflow:
                            // std::cout << "TRADE -> OVERFLOW\n";
                            break;

                        case BufferResult::Full:
                            // std::cout << "TRADE -> FULL\n";
                            // have the client stop submitting trades and candles
                            break;
                    }

                    switch (storage.get_db_result())
                    {
                    case DatabaseResults::SUCCESS:
                        storage.drain();
                        break;
                    
                    case DatabaseResults::QUERY_ERROR:
                        break;

                    case DatabaseResults::CONNECTION_ERROR:
                        break;
                    
                    case DatabaseResults::DUPLICATE:
                        break;
                    }

                    break;
                }

                case BinanceMessageType::OrderBookUpdate:
                {
                    // order books are to be added to redis, not timescaleDB
                    auto update = parser.parse_order_book_updates(message);

                    auto result = order_book_reconstructor.apply_update(update);

                    switch (result)
                    {
                        case UpdateResult::Applied:
                            // TESTING
                            // std::cout << "ORDER BOOK UPDATE APPLIED\n";
                            break;

                        case UpdateResult::Ignored:
                            break;

                        case UpdateResult::SequenceGap:
                            recover_order_book();
                            break;
                    }

                    break;
                }


            } 
        }
        catch (const std::exception& e)
        {
            std::cerr << "Market data error: " << e.what() << '\n';
        }
    }
}