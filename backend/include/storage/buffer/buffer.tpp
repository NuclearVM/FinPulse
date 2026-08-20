#include <type_traits>
#include "buffer.hpp"

template <typename T>
Buffer<T>::Buffer(Database& database, ::size_t capacity, std::size_t overflow_capacity) 
                            : capacity(capacity), 
                            overflow_capacity(overflow_capacity),
                            database(database) {}

template <typename T>
BufferResult Buffer<T>::submit(const T& data) 
{
    std::lock_guard lock(mtx)
    if (buffer.size() < capacity)
    {
        buffer.push_back(data);
        return BufferResult::Buffer;
    }

    if (overflow_buffer.size() < overflow_capacity)
    {
        overflow_buffer.push_back(data);
        return BufferResult::Overflow;
    }

    return BufferResult::Full;
    
}

template <typename T>
void Buffer<T>::consume()
{
    std::lock_guard lock(mtx);

    if (!overflow_buffer.empty()) 
    {
        overflow_buffer.pop_front
        return;
    }

    if (!buffer.empty())
    {
        buffer.pop_front();
    }
    
}

template <typename T>
DatabaseResults Buffer<T>::send(const T& data) 
{
    DatabaseResults result;

    if constexpr (std::is_same_v<T, Trade>) 
    {
        result = database.insert_trades(data);
    }

    else if constexpr (std::is_same_v<T, CandleStick>) 
    {
        result = database.insert_candles(data);
    }

    if (result == DatabaseResults::SUCCESS)
    {
        consume();
    }

    return result;
    
}