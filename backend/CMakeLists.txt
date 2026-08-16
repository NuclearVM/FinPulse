cmake_minimum_required(VERSION 3.23)

project(FinPulse LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libpqxx REQUIRED)

add_executable(finpulse_backend)

target_sources(finpulse_backend
    PRIVATE
        src/main.cpp
        include/ingestion/binance/binance_client.hpp
        include/models/trade.hpp
        include/ingestion/market_ingestor.hpp
        include/storage/database.hpp
        include/common/config.hpp
        include/common/logger.hpp
        src/storage/database.cpp
        src/common/logger.cpp
        src/ingestion/binance/binance_client.cpp
        include/ingestion/binance/binance_parser.hpp
        src/ingestion/binance/binance_parser.cpp
        include/models/candle_stick.hpp
        include/models/metrics/candle_stick_metrics.hpp
        include/analysis/candle_metrics_calculator.hpp
        src/analysis/candle_metrics_calculator.cpp
        include/models/metrics/volatility_metrics.hpp
        include/analysis/volatility/atr.hpp
        src/analysis/volatility/atr.cpp
        include/analysis/volatility_metrics_calculator.hpp
        src/analysis/volatility_metrics_calculator.cpp
        include/models/order_book/order_book_level.hpp
        include/models/order_book/order_book.hpp
        include/ingestion/binance/order_book_reconstructor.hpp
        include/ingestion/binance/updates/binance_order_book_updates.hpp
        src/ingestion/binance/order_book_reconstructor.cpp
        include/ingestion/binance/updates/binance_order_book_snapshot.hpp
        include/ingestion/binance/updates/update_results.hpp
        include/ingestion/binance/snapshot_retriever.hpp
        src/ingestion/binance/snapshot_retriever.cpp
        include/common/http_client.hpp
        src/common/http_client.cpp
        include/aggregation/candle_aggregator.hpp
        src/aggregation/candle_aggregator.cpp
        include/ingestion/binance/binance_message_type.hpp
        src/common/config.cpp
)

target_include_directories(finpulse_backend
    PRIVATE
        include
)

target_link_libraries(finpulse_backend
    PRIVATE
        Boost::boost
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        libpqxx::pqxx
)