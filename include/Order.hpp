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

enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED
};

struct Order {
    uint64_t id;
    std::string instrument;

    Side side;
    OrderType type;

    int64_t price;
    uint64_t quantity;

    uint64_t sequence;

    // Tracks how much of the original order is still unfilled.
    uint64_t remainingQuantity;

    // Tracks the current lifecycle state of the order.
    OrderStatus status;
};