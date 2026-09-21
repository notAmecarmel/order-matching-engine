#include "OrderBook.hpp"

#include <algorithm>
#include <iostream>

std::vector<Trade> OrderBook::addOrder(const Order& order)
{
    std::vector<Trade> trades;

    if (order.side == Side::BUY) {

        uint64_t remainingQuantity = order.quantity;

        while (
            remainingQuantity > 0 &&
            !sellOrders.empty()
        ) {

            auto bestSell = sellOrders.begin();

            int64_t sellPrice = bestSell->first;

            // BUY cannot match the cheapest SELL
            if (
                order.type == OrderType::LIMIT &&
                order.price < sellPrice
            ) {
                break;
            }

            auto& sellQueue = bestSell->second;

            while (
                remainingQuantity > 0 &&
                !sellQueue.empty()
            ) {

                Order& sellOrder = sellQueue.front();

                uint64_t tradeQuantity =
                    std::min(
                        remainingQuantity,
                        sellOrder.quantity
                    );

                Trade trade{
                    order.id,
                    sellOrder.id,
                    order.instrument,
                    sellOrder.price,
                    tradeQuantity
                };

                trades.push_back(trade);

                remainingQuantity -= tradeQuantity;
                sellOrder.quantity -= tradeQuantity;

                if (sellOrder.quantity == 0) {
                    sellQueue.pop_front();
                }
            }

            if (sellQueue.empty()) {
                sellOrders.erase(bestSell);
            }
        }

        // If the BUY wasn't completely filled,
        // store the remaining quantity in the book.
        if (
            remainingQuantity > 0 &&
            order.type == OrderType::LIMIT
        ) {

            Order remainingOrder = order;

            remainingOrder.quantity = remainingQuantity;

            buyOrders[
                remainingOrder.price
            ].push_back(remainingOrder);
        }
    }

    return trades;
}


void OrderBook::printBook() const
{
    std::cout << "\n========== ORDER BOOK ==========\n";

    std::cout << "\nSELL ORDERS:\n";

    for (const auto& [price, orders] : sellOrders) {

        std::cout << "Price " << price << ": ";

        for (const auto& order : orders) {

            std::cout
                << "[Order "
                << order.id
                << ", Qty "
                << order.quantity
                << "] ";
        }

        std::cout << "\n";
    }

    std::cout << "\nBUY ORDERS:\n";

    for (const auto& [price, orders] : buyOrders) {

        std::cout << "Price " << price << ": ";

        for (const auto& order : orders) {

            std::cout
                << "[Order "
                << order.id
                << ", Qty "
                << order.quantity
                << "] ";
        }

        std::cout << "\n";
    }

    std::cout << "\n================================\n";
}