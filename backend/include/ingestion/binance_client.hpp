#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include "market_ingestor.hpp"

using WebSocket = boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;
    
class BinanceClient : public MarketIngestor {
private:

    boost::asio::io_context io_context;

    boost::asio::ssl::context ssl_context;

    boost::asio::ip::tcp::resolver resolver;

    WebSocket websocket;

    // boost::asio::ip::tcp::socket socket;

public:

    BinanceClient();

    void connect() override;

    void disconnect() override;

    void start() override;

};
