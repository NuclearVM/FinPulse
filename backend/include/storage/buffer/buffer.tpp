#include <type_traits>

template <typename T>
Buffer<T>::Buffer(Database& database, ::size_t capacity, std::size_t overflow_capacity) 
                            : capacity(capacity), 
                            overflow_capacity(overflow_capacity),
                            database(database) {}

template <typename T>
BufferResult Buffer<T>::submit(const T& data) 
{
    std::lock_guard lock(mtx);

    if (buffer.size() < capacity && !overflow_buffer.empty()) 
    {
        buffer_result = BufferResult::Overflow;
        to_buffer();
    }

    else if (buffer.size() < capacity && overflow_buffer.empty())
    {

        buffer.push_back(data);
        buffer_result = BufferResult::Buffer;
        std::cout << "BUFFER\n";
        // send(data);

        // test
        DatabaseResults result = send(data);
        std::cout << "DATABASE RESULT: "
          << static_cast<int>(result)
          << '\n';
        
    }

    else if (overflow_buffer.size() < overflow_capacity)
    {
        overflow_buffer.push_back(data);
        buffer_result = BufferResult::Overflow;
        std::cout << "overflow\n";
    }

    else buffer_result = BufferResult::Full; 

    return buffer_result;
    
}

template <typename T>
void Buffer<T>::consume()
{
    std::lock_guard lock(mtx);
    consume_unlocked();
}

template <typename T>
void Buffer<T>::consume_unlocked()
{

    if (!overflow_buffer.empty()) 
    {
        overflow_buffer.pop_front();
        return;
    }

    else if (!buffer.empty())
    {
        buffer.pop_front();
    }
    
}

template <typename T>
void Buffer<T>:: to_buffer() 
{
    while (buffer_result == BufferResult::Overflow && buffer.size() < capacity)
    {
        buffer.push_back(overflow_buffer.front());
        consume_unlocked();

        if (overflow_buffer.empty())
        {
            buffer_result = BufferResult::Buffer;
        }
    }
    
}

template <typename T>
DatabaseResults Buffer<T>::send(const T& data) 
{
    DatabaseResults result;

    if constexpr (std::is_same_v<T, Trade>) 
    {
        if (overflow_buffer.empty())
        {
            result = database.insert_trades(data);
        }
        
    }

    else if constexpr (std::is_same_v<T, CandleStick>) 
    {
        result = database.insert_candles(data);
    }

    if (result == DatabaseResults::SUCCESS)
    {
        consume_unlocked();
    }

    return result;
    
}
