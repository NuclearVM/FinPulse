#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/ssl.hpp>
#include "ingestion/market_ingestor.hpp"
#include "ingestion/binance/binance_parser.hpp"

using WebSocket = boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;
    
class BinanceClient : public MarketIngestor {
private:

    boost::asio::io_context io_context;

    boost::asio::ssl::context ssl_context;

    boost::asio::ip::tcp::resolver resolver;

    WebSocket websocket;

    // void emit(const Trade& trade);

    std::string read_message();

    // Trade parse_trade(const std::string& message);

    BinanceParser parser;

    bool connected = false;

    boost::beast::flat_buffer buffer;

    // boost::asio::ip::tcp::socket socket;

public:

    BinanceClient();

    void connect() override;

    void disconnect() override;

    void start() override;

};
