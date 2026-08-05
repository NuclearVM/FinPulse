#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket.hpp>

class BinanceClient {

private:
    boost::asio::io_context io_context;

    boost::asio::ssl::context ssl_context;

    boost::asio::ip::tcp::resolver resolver;

    boost::asio::ip::tcp::socket socket;

public:
    BinanceClient();

    void connect();

};
