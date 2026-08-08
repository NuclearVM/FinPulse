#include <iostream>
#include <iomanip>
#include "ingestion/binance/binance_client.hpp"
#include "models/trade.hpp"
#include "ingestion/binance/order_book_reconstructor.hpp"


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
    // BinanceClient client;
    // std::cout << "Client created\n";

    // client.connect();

    // Trade trade;

    // BinanceClient client;

    // BinanceOrderBookUpdate update;

    // update.symbol = "BTCUSDT";

    // update.bids = {
    //     {100.0, 2.0},
    //     {99.0, 5.0},
    //     {98.0, 10.0}
    // };

    // update.asks = {
    //     {101.0, 3.0},
    //     {102.0, 7.0},
    //     {103.0, 12.0}
    // };

    // //  fake test trade
    // // client.set_trade_callback(
    // //     [](const Trade& trade)
    // //     {
    // //         std::cout
    // //             << trade.symbol
    // //             << " "
    // //             << std::fixed
    // //             << std::setprecision(8)
    // //             << trade.price
    // //             << " "
    // //             << trade.quantity
    // //             << '\n';
    // //     }
    // // );

    // // client.connect();

    // // client.start();

    // OrderBookReconstructor reconstructor;

    // reconstructor.apply_update(update);

    // OrderBook book = reconstructor.get_order_book();

    //  std::cout << "Symbol: " << book.symbol << '\n';

    // std::cout << "\nBids:\n";

    // for (const auto& level : book.bids)
    // {
    //     std::cout
    //         << level.price
    //         << " -> "
    //         << level.quantity
    //         << '\n';
    // }

    // std::cout << "\nAsks:\n";

    // for (const auto& level : book.asks)
    // {
    //     std::cout
    //         << level.price
    //         << " -> "
    //         << level.quantity
    //         << '\n';
    // }

    // BinanceOrderBookUpdate update3;

    // update3.symbol = "BTCUSDT";

    // update3.bids = {
    //     {99.0, 0.0}
    // };

    // update3.asks = {
    //     {102.0, 0.0}
    // };

    // book = reconstructor.get_order_book();

    // reconstructor.apply_update(update3);
    
    // ORDER BOOK TEST

    OrderBookReconstructor reconstructor;

    // --------------------------------------------------
    // Test 1: Insert initial price levels
    // --------------------------------------------------

    BinanceOrderBookUpdate update;

    update.symbol = "BTCUSDT";

    update.bids = {
        {100.0, 2.0},
        {99.0, 5.0},
        {98.0, 10.0}
    };

    update.asks = {
        {101.0, 3.0},
        {102.0, 7.0},
        {103.0, 12.0}
    };

    reconstructor.apply_update(update);

    OrderBook book = reconstructor.get_order_book();

    std::cout << "=== Initial Book ===\n";
    print_order_book(book);


    // --------------------------------------------------
    // Test 2: Update existing price levels
    // --------------------------------------------------

    BinanceOrderBookUpdate update2;

    update2.symbol = "BTCUSDT";

    update2.bids = {
        {100.0, 8.0}
    };

    update2.asks = {
        {101.0, 6.0}
    };

    reconstructor.apply_update(update2);

    book = reconstructor.get_order_book();

    std::cout << "\n=== After Modification ===\n";
    print_order_book(book);


    // --------------------------------------------------
    // Test 3: Delete price levels
    // Binance uses quantity = 0 to remove a level.
    // --------------------------------------------------

    BinanceOrderBookUpdate update3;

    update3.symbol = "BTCUSDT";

    update3.bids = {
        {99.0, 0.0}
    };

    update3.asks = {
        {102.0, 0.0}
    };

    reconstructor.apply_update(update3);

    book = reconstructor.get_order_book();

    std::cout << "\n=== After Deletion ===\n";
    print_order_book(book);

    return 0;
}


