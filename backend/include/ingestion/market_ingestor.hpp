#pragma once


class MarketIngestor
{

public:

    virtual void connect() = 0;

    virtual void disconnect() = 0;

    virtual void start() = 0;

    virtual ~MarketIngestor() = default;

};