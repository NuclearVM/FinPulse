#pragma once

#include <iostream>
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

    BufferResult buffer_result;
    DatabaseResults db_result;

    Database& database;

    std::mutex mtx;

    void consume();

    void consume_unlocked();

    DatabaseResults send(const T& data);

    void to_buffer();

public:
    Buffer(Database& database, std::size_t capacity, std::size_t overflow_capacity);

    BufferResult submit(const T& data);

    void drain();

    DatabaseResults get_db_result() const;

    // void consume();
    // void consume_unlocked();

    // DatabaseResults send(const T& data);
};

#include "buffer.tpp"