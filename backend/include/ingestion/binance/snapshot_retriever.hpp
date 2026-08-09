#pragma once

#include <cstddef>
#include <string>

#include "common/http_client.hpp"
#include "ingestion/binance/binance_parser.hpp"
#include "models/order_book/order_book.hpp"

class BinanceOrderBookSnapshotRetriever {

private:

    BinanceParser& parser;
    HttpClient& http_client;

public:

    explicit BinanceOrderBookSnapshotRetriever(HttpClient& http_client, BinanceParser& parser);

    BinanceOrderBookSnapshot get_snapshot(const std::string& symbol, std::size_t limit);
};