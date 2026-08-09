#include "common/http_client.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <stdexcept>

HttpClient::HttpClient(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context) : io_context(io_context), ssl_context(ssl_context) {}

std::string HttpClient::get(const std::string& host, const std::string& target)
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;

    asio::ip::tcp::resolver resolver(io_context);

    auto endpoints = resolver.resolve(host, "443");

    beast::ssl_stream<asio::ip::tcp::socket> stream(io_context, ssl_context);

    asio::connect(stream.next_layer(), endpoints);

    stream.handshake(asio::ssl::stream_base::client);

    http::request<http::empty_body> request{http::verb::get, target, 11};

    request.set(http::field::host, host);

    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, request);

    beast::flat_buffer buffer;

    http::response<http::string_body> response;

    http::read(stream, buffer, response);

    beast::error_code shutdown_error;

    stream.shutdown(shutdown_error);

    if (response.result() != http::status::ok)
    {
        throw std::runtime_error("HTTP GET request failed: " + std::string(response.reason()));
    }

    return response.body();
}