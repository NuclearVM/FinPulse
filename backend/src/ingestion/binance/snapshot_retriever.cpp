#include "ingestion/binance/snapshot_retriever.hpp"

#include <stdexcept>

BinanceOrderBookSnapshotRetriever::BinanceOrderBookSnapshotRetriever(
    HttpClient& http_client,
    BinanceParser& parser) : http_client(http_client), parser(parser) {}

BinanceOrderBookSnapshot BinanceOrderBookSnapshotRetriever::get_snapshot(const std::string& symbol, std::size_t limit)
{
    if (symbol.empty())
    {
        throw std::invalid_argument(
            "Order book snapshot symbol cannot be empty"
        );
    }

    if (limit == 0)
    {
        throw std::invalid_argument(
            "Order book snapshot limit must be greater than zero"
        );
    }

    const std::string target = "/api/v3/depth?symbol=" + symbol + "&limit=" + std::to_string(limit);

    const std::string response = http_client.get("api.binance.com", target);

    return parser.parse_order_book_snapshot(response, symbol);
}