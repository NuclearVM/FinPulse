#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

class HttpClient
{
private:
    boost::asio::io_context& io_context;
    boost::asio::ssl::context& ssl_context;

public:
    HttpClient(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context);

    std::string get(const std::string& host, const std::string& target);
};
