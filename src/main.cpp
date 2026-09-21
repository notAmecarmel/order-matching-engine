#include "OrderBook.hpp"

#include <iostream>

int main()
{
    OrderBook book;

    Order sellOrder{
        1,
        "AAPL",
        Side::SELL,
        OrderType::LIMIT,
        10000,
        100,
        1
    };

    Order buyOrder{
        2,
        "AAPL",
        Side::BUY,
        OrderType::LIMIT,
        10100,
        75,
        2
    };

    book.addOrder(sellOrder);

    std::cout << "\nBefore matching:";
    book.printBook();

    auto trades = book.addOrder(buyOrder);

    std::cout << "\nTrades generated:\n";

    for (const auto& trade : trades) {

        std::cout
            << "BUY Order "
            << trade.buyOrderId
            << " matched SELL Order "
            << trade.sellOrderId
            << " | Price: "
            << trade.price
            << " | Quantity: "
            << trade.quantity
            << "\n";
    }

    std::cout << "\nAfter matching:";
    book.printBook();

    return 0;
}