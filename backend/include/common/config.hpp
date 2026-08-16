#pragma once

#include <string>

struct DatabaseConfig
{
    std::string host;
    std::string port;
    std::string database;
    std::string user;
    std::string password;
};

DatabaseConfig load_database_config();