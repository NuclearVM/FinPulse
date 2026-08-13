#include "ingestion/binance/order_book_reconstructor.hpp"

void OrderBookReconstructor::initialize( const BinanceOrderBookSnapshot& snapshot)
{
    bids.clear();
    asks.clear();

    symbol = snapshot.symbol;
    last_update_id = snapshot.last_update_id;

    for (const auto& level : snapshot.bids)
    {
        bids[level.price] = level.quantity;
    }

    for (const auto& level : snapshot.asks)
    {
        asks[level.price] = level.quantity;
    }
}

UpdateResult OrderBookReconstructor::apply_update(const BinanceOrderBookUpdate& update)
{
    // ignore updates that are older than the current state

    if (update.final_update_id <= last_update_id) {return UpdateResult::Ignored; }

    // check if there's any gaps, if gaps are foud than the current book can't be trusted

    if (update.first_update_id > last_update_id + 1) {return UpdateResult::SequenceGap; }

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

    last_update_id = update.final_update_id;

    return UpdateResult::Applied;
}

OrderBook OrderBookReconstructor::get_order_book() const
{
    OrderBook book;

    book.symbol = symbol;
    book.timestamp = timestamp;

    book.bids.reserve(bids.size());
    book.asks.reserve(asks.size());

    // maybe change these later instead of calling order book on every frame
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