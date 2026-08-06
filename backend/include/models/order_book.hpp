#pragma once

#include <map>

// still a work in progress
// this is not the final model

struct OrderBook
{
    std::string symbol;

    std::map<double,double> bids;
    std::map<double,double> asks;
};