#include "binance_client.hpp"

BinanceClient::BinanceClient() : ssl_context(boost::asio::ssl::context::tls_client), resolver(io_context), socket(io_context) {

}

void BinanceClient::connect() {

}