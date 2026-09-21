#pragma once

#include <cstdint>
#include <string>

enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    LIMIT,
    MARKET
};

struct Order {
    uint64_t id;
    std::string instrument;

    Side side;
    OrderType type;

    int64_t price;
    uint64_t quantity;

    uint64_t sequence;
};