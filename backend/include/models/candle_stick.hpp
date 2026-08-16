#pragma once
#include <string>
#include <chrono>
#include <cstdint>

struct CandleStick 
{
    std::string symbol;
    std::string exchange;

    double open;
    double close;
    double high;
    double low;
    double volume;
    std::chrono::system_clock::time_point timestamp;
    
};