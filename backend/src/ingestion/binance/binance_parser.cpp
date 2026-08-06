#include "ingestion/binance/binance_parser.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Trade BinanceParser::parse_trade(const std::string& message) {

    auto data = json::parse(message);

    Trade trade;

    trade.exchange = "Binance";

    trade.symbol = data["s"].get<std::string>();

    trade.price = std::stod(data["p"].get<std::string>());

    trade.quantity = std::stod(data["q"].get<std::string>());

    trade.trade_id = data["t"].get<uint64_t>();

    trade.buyer_is_maker = data["m"].get<bool>();

    trade.event_time = convert_timestamp(data["E"]);

    trade.execution_time = convert_timestamp(data["T"]);

    return trade;
}

// performs conversion
std::chrono::system_clock::time_point BinanceParser::convert_timestamp(uint64_t timestamp) {

    // return std::chrono::system_clock::time_point(std::chrono::milliseconds(timestamp));
    std::chrono::milliseconds duration(timestamp);

    std::chrono::system_clock::time_point time(duration);

    return time;
}