-- Enable TimescaleDB
CREATE EXTENSION IF NOT EXISTS timescaledb;

-- Trades

CREATE TABLE trades (
    trade_id        BIGINT NOT NULL,
    exchange        TEXT NOT NULL,
    symbol          TEXT NOT NULL,
    price           DOUBLE PRECISION NOT NULL,
    quantity        DOUBLE PRECISION NOT NULL,
    buyer_is_maker  BOOLEAN NOT NULL,
    event_time      TIMESTAMPTZ NOT NULL,
    execution_time  TIMESTAMPTZ NOT NULL,
    received_time   TIMESTAMPTZ NOT NULL,

    PRIMARY KEY (exchange, symbol, trade_id, event_time)
);

SELECT create_hypertable(
    'trades',
    by_range('event_time'),
    if_not_exists => TRUE
);

CREATE INDEX idx_trades_symbol_time
    ON trades (symbol, event_time DESC);

-- Automatically remove trades older than 90 days.

SELECT add_retention_policy(
    'trades',
    INTERVAL '90 days' 
);


-- Candles

CREATE TABLE candles (
    exchange        TEXT NOT NULL,
    symbol          TEXT NOT NULL,
    candle_time     TIMESTAMPTZ NOT NULL,

    open_price      DOUBLE PRECISION NOT NULL,
    high            DOUBLE PRECISION NOT NULL,
    low             DOUBLE PRECISION NOT NULL,
    close_price     DOUBLE PRECISION NOT NULL,
    volume          DOUBLE PRECISION NOT NULL,

    PRIMARY KEY (exchange, symbol, candle_time)
);

SELECT create_hypertable(
    'candles',
    by_range('candle_time'),
    if_not_exists => TRUE
);

CREATE INDEX idx_candles_symbol_time
    ON candles (symbol, candle_time DESC);