enum class BufferResult
{
    Buffer,         // call when buffer has space
    Overflow,       // call when the oveflow buffer has space
    Full            // call when both buffers are full needs to be used in client
};