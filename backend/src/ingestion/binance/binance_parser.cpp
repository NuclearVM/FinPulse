#include "ingestion/binance/binance_parser.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BinanceMessageType BinanceParser::identify_message(const std::string& message) 
{
    const auto data = json::parse(message).at("data");

    const auto event_type = data.at("e").get<std::string>();

    if (event_type == "trade") return BinanceMessageType::Trade;

    if (event_type == "depthUpdate") return BinanceMessageType::OrderBookUpdate;

    throw std::runtime_error("Unknown Binance message type: " + event_type);
}

Trade BinanceParser::parse_trade(const std::string& message) {

    auto data = json::parse(message).at("data");

    Trade trade;

    trade.exchange = "Binance";

    trade.symbol = data["s"].get<std::string>();

    trade.price = std::stod(data["p"].get<std::string>());

    trade.quantity = std::stod(data["q"].get<std::string>());

    trade.trade_id = data["t"].get<uint64_t>();

    trade.buyer_is_maker = data["m"].get<bool>();

    trade.event_time = convert_timestamp(data["E"]);

    trade.execution_time = convert_timestamp(data["T"]);

    trade.received_time = std::chrono::system_clock::now();

    return trade;
}

// performs conversion
std::chrono::system_clock::time_point BinanceParser::convert_timestamp(uint64_t timestamp) {

    // return std::chrono::system_clock::time_point(std::chrono::milliseconds(timestamp));
    std::chrono::milliseconds duration(timestamp);

    std::chrono::system_clock::time_point time(duration);

    return time;
}

BinanceOrderBookUpdate BinanceParser::parse_order_book_updates(const std::string& message) {

    const auto data = json::parse(message).at("data");

    BinanceOrderBookUpdate update;

    update.symbol = data.at("s").get<std::string>();

    update.first_update_id = data.at("U").get<std::uint64_t>();

    update.final_update_id = data.at("u").get<std::uint64_t>();

    update.timestamp = convert_timestamp(data.at("E").get<std::uint64_t>());

    for (const auto& bid : data.at("b"))
    {
        update.bids.push_back(
            OrderBookLevel{
                std::stod(bid.at(0).get<std::string>()),
                std::stod(bid.at(1).get<std::string>())
            }
        );
    }

     for (const auto& ask : data.at("a"))
    {
        update.asks.push_back(
            OrderBookLevel{
                std::stod(ask.at(0).get<std::string>()),
                std::stod(ask.at(1).get<std::string>())
            }
        );
    }

    return update;  

}

BinanceOrderBookSnapshot BinanceParser::parse_order_book_snapshot(const std::string& message, const std::string& symbol)
{
    const auto data = nlohmann::json::parse(message);

    BinanceOrderBookSnapshot snapshot;

    snapshot.symbol = symbol;

    snapshot.last_update_id = data.at("lastUpdateId").get<std::uint64_t>();

    for (const auto& level : data.at("bids"))
    {
        snapshot.bids.push_back(
            OrderBookLevel{
                std::stod(level.at(0).get<std::string>()),
                std::stod(level.at(1).get<std::string>())
            }
        );
    }

    for (const auto& level : data.at("asks"))
    {
        snapshot.asks.push_back(
            OrderBookLevel{
                std::stod(level.at(0).get<std::string>()),
                std::stod(level.at(1).get<std::string>())
            }
        );
    }

    return snapshot;
}