#include "OrderBook.hpp"

#include <algorithm>
#include <iostream>


// ============================================================
// ADD ORDER
// ============================================================
//
// This is the main entry point into the order book.
//
// We receive an order and:
//   1. Try to match it against existing orders.
//   2. Generate Trade objects for every match.
//   3. If the order isn't completely filled, put the remainder
//      into the order book.
//
// ============================================================

std::vector<Trade> OrderBook::addOrder(const Order& order)
{
    std::vector<Trade> trades;

    // ========================================================
    // BUY ORDER
    // ========================================================
    //
    // A BUY can match against SELL orders when:
    //
    //     BUY price >= SELL price
    //
    // Example:
    //
    //     BUY  @ 101
    //     SELL @ 100
    //
    //     101 >= 100  -> MATCH
    //
    // SELL prices are sorted from LOW -> HIGH.
    // Therefore sellOrders.begin() gives us the cheapest seller.
    //
    // ========================================================

    if (order.side == Side::BUY)
    {
        uint64_t remainingQuantity = order.quantity;

        while (
            remainingQuantity > 0 &&
            !sellOrders.empty()
        )
        {
            // Get the cheapest SELL price level.
            auto bestSell = sellOrders.begin();

            // .first = price
            // .second = queue of orders at that price
            int64_t sellPrice = bestSell->first;

            // For a LIMIT order, stop if the seller is too expensive.
            //
            // Because SELL prices are sorted ascending, if the
            // cheapest seller can't match, nobody else can either.
            if (
                order.type == OrderType::LIMIT &&
                order.price < sellPrice
            )
            {
                break;
            }

            // Get the actual queue of orders at this price.
            // '&' means we reference the real queue in the book,
            // rather than creating a copy.
            auto& sellQueue = bestSell->second;

            // Process orders at this price in FIFO order.
            while (
                remainingQuantity > 0 &&
                !sellQueue.empty()
            )
            {
                // Get the first order in the queue.
                // This gives us price-time priority.
                Order& sellOrder = sellQueue.front();

                // We can only trade the smaller of:
                //
                //   buyer's remaining quantity
                //   seller's available quantity
                //
                uint64_t tradeQuantity =
                    std::min(
                        remainingQuantity,
                        sellOrder.quantity
                    );

                // Create a record of the transaction.
                Trade trade{
                    order.id,
                    sellOrder.id,
                    order.instrument,
                    sellOrder.price,
                    tradeQuantity
                };

                trades.push_back(trade);

                // Reduce the quantity remaining on both sides.
                remainingQuantity -= tradeQuantity;
                sellOrder.quantity -= tradeQuantity;

                // If seller is completely filled,
                // remove it from the front of the FIFO queue.
                if (sellOrder.quantity == 0)
                {
                    sellQueue.pop_front();
                }
            }

            // If there are no orders left at this price,
            // remove the empty price level.
            if (sellQueue.empty())
            {
                sellOrders.erase(bestSell);
            }
        }

        // If the BUY wasn't completely filled,
        // store the remaining quantity in the BUY book.
        //
        // Market orders are NOT stored because a market order
        // has no resting price.
        if (
            remainingQuantity > 0 &&
            order.type == OrderType::LIMIT
        )
        {
            Order remainingOrder = order;

            remainingOrder.quantity = remainingQuantity;

            buyOrders[
                remainingOrder.price
            ].push_back(remainingOrder);
        }
    }


    // ========================================================
    // SELL ORDER
    // ========================================================
    //
    // This is the mirror image of the BUY logic.
    //
    // A SELL can match against BUY orders when:
    //
    //     SELL price <= BUY price
    //
    // BUY prices are sorted HIGH -> LOW.
    // Therefore buyOrders.begin() gives us the highest buyer.
    //
    // ========================================================

    else
    {
        uint64_t remainingQuantity = order.quantity;

        while (
            remainingQuantity > 0 &&
            !buyOrders.empty()
        )
        {
            // Get the highest BUY price level.
            auto bestBuy = buyOrders.begin();

            // .first = price
            int64_t buyPrice = bestBuy->first;

            // For a LIMIT SELL, stop if the buyer is
            // offering too little.
            //
            // Because BUY prices are sorted descending,
            // if the highest buyer can't match,
            // nobody else can either.
            if (
                order.type == OrderType::LIMIT &&
                order.price > buyPrice
            )
            {
                break;
            }

            // Get the FIFO queue at this price.
            auto& buyQueue = bestBuy->second;

            // Process orders from oldest to newest.
            while (
                remainingQuantity > 0 &&
                !buyQueue.empty()
            )
            {
                // First BUY order at this price.
                Order& buyOrder = buyQueue.front();

                // Determine how many units can actually trade.
                uint64_t tradeQuantity =
                    std::min(
                        remainingQuantity,
                        buyOrder.quantity
                    );

                // Create the trade.
                Trade trade{
                    buyOrder.id,
                    order.id,
                    order.instrument,
                    buyOrder.price,
                    tradeQuantity
                };

                trades.push_back(trade);

                // Reduce both quantities.
                remainingQuantity -= tradeQuantity;
                buyOrder.quantity -= tradeQuantity;

                // Remove completely filled BUY order.
                if (buyOrder.quantity == 0)
                {
                    buyQueue.pop_front();
                }
            }

            // Remove empty price level.
            if (buyQueue.empty())
            {
                buyOrders.erase(bestBuy);
            }
        }

        // If SELL still has quantity remaining,
        // add it to the SELL book.
        if (
            remainingQuantity > 0 &&
            order.type == OrderType::LIMIT
        )
        {
            Order remainingOrder = order;

            remainingOrder.quantity = remainingQuantity;

            sellOrders[
                remainingOrder.price
            ].push_back(remainingOrder);
        }
    }

    return trades;
}

bool OrderBook::cancelOrder(
    uint64_t orderId,
    Side side,
    int64_t price
)
{
    // Find the appropriate price level.
    //
    // BUY -> buyOrders
    // SELL -> sellOrders

    if (side == Side::BUY)
    {
        auto priceLevel = buyOrders.find(price);

        // Price doesn't exist.
        if (priceLevel == buyOrders.end())
        {
            return false;
        }

        auto& queue = priceLevel->second;

        // Search only this price level.
        for (auto it = queue.begin();
             it != queue.end();
             ++it)
        {
            if (it->id == orderId)
            {
                // Remove the order from the FIFO queue.
                queue.erase(it);

                // If no orders remain at this price,
                // remove the price level too.
                if (queue.empty())
                {
                    buyOrders.erase(priceLevel);
                }

                return true;
            }
        }
    }

    else
    {
        auto priceLevel = sellOrders.find(price);

        if (priceLevel == sellOrders.end())
        {
            return false;
        }

        auto& queue = priceLevel->second;

        for (auto it = queue.begin();
             it != queue.end();
             ++it)
        {
            if (it->id == orderId)
            {
                queue.erase(it);

                if (queue.empty())
                {
                    sellOrders.erase(priceLevel);
                }

                return true;
            }
        }
    }

    // Order wasn't found.
    return false;
}

// ============================================================
// PRINT ORDER BOOK
// ============================================================

void OrderBook::printBook() const
{
    std::cout << "\n========== ORDER BOOK ==========\n";

    std::cout << "\nSELL ORDERS:\n";

    for (const auto& [price, orders] : sellOrders)
    {
        std::cout << "Price " << price << ": ";

        for (const auto& order : orders)
        {
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

    for (const auto& [price, orders] : buyOrders)
    {
        std::cout << "Price " << price << ": ";

        for (const auto& order : orders)
        {
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