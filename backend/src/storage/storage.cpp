#include "storage/storage.hpp"

Storage::Storage(const DatabaseConfig& config)
                                : database(config),
                                trade_buffer(database, 10000, 50000),
                                candle_buffer(database, 7000, 30000) {}