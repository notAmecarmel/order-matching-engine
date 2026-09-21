#pragma once

#include "Order.hpp"
#include "Trade.hpp"

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <vector>

class OrderBook
{
public:

    std::vector<Trade> addOrder(
        const Order& order
    );

    bool cancelOrder(
        uint64_t orderId,
        Side side,
        int64_t price
    );

    void printBook() const;

private:

    using OrderQueue = std::deque<Order>;

    std::map<
        int64_t,
        OrderQueue,
        std::greater<int64_t>
    > buyOrders;

    std::map<
        int64_t,
        OrderQueue
    > sellOrders;
};