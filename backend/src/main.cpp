#include <iostream>
#include <iomanip>
#include "ingestion/binance/binance_client.hpp"
#include "models/trade.hpp"
#include "ingestion/binance/order_book_reconstructor.hpp"
#include "common/http_client.hpp"
#include "ingestion/binance/snapshot_retriever.hpp"


void print_order_book(const OrderBook& book)
{
    std::cout << "Symbol: " << book.symbol << '\n';

    std::cout << "\nBids:\n";

    for (const auto& level : book.bids)
    {
        std::cout
            << level.price
            << " -> "
            << level.quantity
            << '\n';
    }

    std::cout << "\nAsks:\n";

    for (const auto& level : book.asks)
    {
        std::cout
            << level.price
            << " -> "
            << level.quantity
            << '\n';
    }
}

int main() {

    boost::asio::io_context io_context;

    boost::asio::ssl::context ssl_context{boost::asio::ssl::context::tls_client};

    HttpClient http_client(io_context, ssl_context);

    BinanceOrderBookSnapshotRetriever snapshot_retriever(http_client);

    std::string response = snapshot_retriever.get_snapshot("BTCUSDT", 100);

    std::cout << response<< '\n';

    return 0;
}
