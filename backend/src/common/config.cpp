#include "common/config.hpp"

#include <cstdlib>
#include <stdexcept>

namespace 
{
    std::string get_required_env(const char* name)
    {
        const char* value = std::getenv(name);

        if (value == nullptr || *value == '\0')
        {
            throw std::runtime_error(
                std::string("Missing required environment variable: ") + name
            );
        }

        return value;
    }
}

DatabaseConfig load_database_config()
{
    return DatabaseConfig{
        .host = get_required_env("POSTGRES_HOST"),
        .port = get_required_env("POSTGRES_PORT"),
        .database = get_required_env("POSTGRES_DB"),
        .user = get_required_env("POSTGRES_USER"),
        .password = get_required_env("POSTGRES_PASSWORD")
    };
}