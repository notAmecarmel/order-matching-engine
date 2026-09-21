#pragma once

#include "OrderBook.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct OrderLocation
{
    std::string instrument;
    Side side;
    int64_t price;
};

class MatchingEngine
{
public:

    std::vector<Trade> submitOrder(
        const Order& order
    );

    bool cancelOrder(
        uint64_t orderId
    );

    void printAllBooks() const;

private:

    std::unordered_map<
        std::string,
        OrderBook
    > books;

    // Fast lookup:
    //
    // order ID -> where the order lives
    //
    std::unordered_map<
        uint64_t,
        OrderLocation
    > orderIndex;
};