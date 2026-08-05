#include <boost/asio.hpp>
#include <iostream>

int main() {
    boost::asio::io_context io;
    std::cout<<"hey I run";

    io.run();
}