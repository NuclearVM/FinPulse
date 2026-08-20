#pragma once

#include <cstddef>
#include <deque>
#include <mutex>

#include "buffer_results.hpp"
#include "storage/database.hpp"
#include "storage/database_results.hpp"

template <typename T> class Buffer {

private:
    std::deque<T> buffer;
    std::deque<T> overflow_buffer;

    std::size_t capacity;
    std::size_t overflow_capacity;

    Database& database;

    std::mutex mtx;

public:
    Buffer(Database& database, std::size_t capacity, std::size_t overflow_capacity);

    BufferResult submit(const T& data);

    void consume();

    DatabaseResults send(const T& data);
};

#include "buffer.tpp"