#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/ssl.hpp>

#include "ingestion/market_ingestor.hpp"
#include "ingestion/binance/binance_parser.hpp"
#include "ingestion/binance/order_book_reconstructor.hpp"
#include "ingestion/binance/snapshot_retriever.hpp"
#include "common/http_client.hpp"

using WebSocket = boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;
    
class BinanceClient : public MarketIngestor {

private:

    boost::asio::io_context io_context;
    boost::asio::ssl::context ssl_context;
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::flat_buffer buffer;
    WebSocket websocket;

    BinanceParser parser;
    OrderBookReconstructor order_book_reconstructor;

    HttpClient http_client;
    BinanceOrderBookSnapshotRetriever snapshot_retriever;

    std::string read_message();

    bool connected = false;

    void initialize_order_book();
    void recover_order_book();

    // BinanceOrderBookSnapshot get_order_book_snapshot(const std::string& symbol, std::size_t limit);

public:

    BinanceClient();

    void connect() override;
    void disconnect() override;
    void start() override;
};
