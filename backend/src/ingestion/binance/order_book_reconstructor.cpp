#include "ingestion/binance/order_book_reconstructor.hpp"

void OrderBookReconstructor::initialize( const BinanceOrderBookSnapshot& snapshot)
{
    bids.clear();
    asks.clear();

    symbol = snapshot.symbol;

    for (const auto& level : snapshot.bids)
    {
        bids[level.price] = level.quantity;
    }

    for (const auto& level : snapshot.asks)
    {
        asks[level.price] = level.quantity;
    }
}

void OrderBookReconstructor::apply_update(const BinanceOrderBookUpdate& update)
{
    symbol = update.symbol;
    timestamp = update.timestamp;

    for (const auto& level : update.bids)
    {
        if (level.quantity == 0.0)
        {
            bids.erase(level.price);
        }
        else
        {
            bids[level.price] = level.quantity;
        }
    }

    for (const auto& level : update.asks)
    {
        if (level.quantity == 0.0)
        {
            asks.erase(level.price);
        }
        else
        {
            asks[level.price] = level.quantity;
        }
    }
}

OrderBook OrderBookReconstructor::get_order_book() const
{
    OrderBook book;

    book.symbol = symbol;
    book.timestamp = timestamp;

    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());

    for (const auto& [price, quantity] : bids)
    {
        book.bids.push_back(OrderBookLevel{price, quantity});
    }

    for (const auto& [price, quantity] : asks)
    {
        book.asks.push_back(OrderBookLevel{price, quantity});
    }

    return book;
}