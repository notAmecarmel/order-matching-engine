#include "MatchingEngine.hpp"

#include <iostream>


std::vector<Trade> MatchingEngine::submitOrder(
    const Order& order
)
{
    auto trades = books[
        order.instrument
    ].addOrder(order);

    // Determine whether the order still has
    // unfilled quantity.
    //
    // If it was completely filled, it shouldn't
    // be cancellable anymore.
    //
    // For the moment we determine this by checking
    // whether the order still exists in the book.
    //
    // We'll improve this design later.

    if (trades.empty())
    {
        // Entire order is resting in the book.
        orderIndex[order.id] = {
            order.instrument,
            order.side,
            order.price
        };
    }
    else
    {
        uint64_t tradedQuantity = 0;

        for (const auto& trade : trades)
        {
            tradedQuantity += trade.quantity;
        }

        if (tradedQuantity < order.quantity)
        {
            // Partial fill.
            //
            // Remaining quantity is resting in the book.
            orderIndex[order.id] = {
                order.instrument,
                order.side,
                order.price
            };
        }
    }

    return trades;
}

bool MatchingEngine::cancelOrder(
    uint64_t orderId
)
{
    // Find where the order is stored.
    auto it = orderIndex.find(orderId);

    if (it == orderIndex.end())
    {
        // Unknown order ID.
        return false;
    }

    const OrderLocation& location = it->second;

    // Find the appropriate instrument book.
    auto bookIt = books.find(
        location.instrument
    );

    if (bookIt == books.end())
    {
        return false;
    }

    // Ask the OrderBook to remove the order.
    bool cancelled = bookIt->second.cancelOrder(
        orderId,
        location.side,
        location.price
    );

    if (cancelled)
    {
        // It no longer exists in the book,
        // so remove it from our index.
        orderIndex.erase(it);
    }

    return cancelled;
}


void MatchingEngine::printAllBooks() const
{
    std::cout << "\n\n========================================\n";
    std::cout << "          ALL ORDER BOOKS\n";
    std::cout << "========================================\n";

    for (const auto& [instrument, book] : books)
    {
        std::cout << "\nInstrument: " << instrument << "\n";

        book.printBook();
    }
}

