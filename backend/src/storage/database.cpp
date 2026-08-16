#include "storage/database.hpp"

#include <stdexcept>
#include <iostream>

Database::Database(const DatabaseConfig& config) : 
                                connection(
                                    "host=" + config.host +
                                    " port=" + config.port +
                                    " dbname=" + config.database +
                                    " user=" + config.user +
                                    " password=" + config.password
                                ) {
    if (!connection.is_open())
    {
        throw std::runtime_error("Failed to open database connection");
    }

    std::cout << "Connected to database\n";
}