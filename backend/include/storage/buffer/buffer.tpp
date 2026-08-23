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
        send(data);
        // std::cout << "BUFFER SIZE: " << buffer.size()
        //   << " | OVERFLOW SIZE: " << overflow_buffer.size()
        //   << '\n';

        // test
        // DatabaseResults result = send(data);
        // std::cout << "DATABASE RESULT: "
        //   << static_cast<int>(result)
        //   << '\n';
        
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
void Buffer<T>::drain()
{
    while (!buffer.empty())
    {
        db_result = send(buffer.front());

        if (db_result == DatabaseResults::SUCCESS)
            continue;

        if (db_result == DatabaseResults::DUPLICATE)
        {
            buffer.pop_front();
            continue;
        }

        break;
    }
}

template <typename T>
DatabaseResults Buffer<T>::send(const T& data) 
{    
    if constexpr (std::is_same_v<T, Trade>) 
    {
        db_result = database.insert_trades(data);
    }

    else if constexpr (std::is_same_v<T, CandleStick>) 
    {
        db_result = database.insert_candles(data);
    }

    if (db_result == DatabaseResults::SUCCESS)
    {
        // std::cout<<"consumed\n";
        consume_unlocked();
    }

    return db_result;
    
}

template <typename T>
DatabaseResults Buffer<T>::get_db_result() const
{
    return db_result;
}

