cmake_minimum_required(VERSION 3.23)

project(FinPulse LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(finpulse_backend)

target_sources(finpulse_backend
    PRIVATE
        src/main.cpp
        include/ingestion/binance_client.hpp
        include/models/trade.hpp
        include/ingestion/market_ingestor.hpp
        include/storage/database.hpp
        include/common/config.hpp
        include/common/logger.hpp
        src/storage/database.cpp
        src/ingestion/binance_client.cpp
        src/common/logger.cpp
        include/models/order_book.hpp
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
)