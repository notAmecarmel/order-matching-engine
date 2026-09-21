#pragma once

#include "Order.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class EventLog
{
public:

    explicit EventLog(const std::string& filename)
        : filename(filename)
    {
    }


    // Write an order event to disk.
    void appendOrder(const Order& order)
    {
        std::ofstream file(
            filename,
            std::ios::app
        );

        file
            << "ORDER "
            << order.id << " "
            << order.instrument << " "
            << static_cast<int>(order.side) << " "
            << static_cast<int>(order.type) << " "
            << order.price << " "
            << order.quantity << " "
            << order.sequence
            << "\n";
    }


    // Read all previously persisted orders.
    std::vector<Order> replay()
    {
        std::vector<Order> orders;

        std::ifstream file(filename);

        std::string line;

        while (std::getline(file, line))
        {
            std::stringstream stream(line);

            std::string eventType;

            stream >> eventType;

            // Currently we only have ORDER events.
            if (eventType != "ORDER")
            {
                continue;
            }

            Order order;

            int side;
            int type;

            stream
                >> order.id
                >> order.instrument
                >> side
                >> type
                >> order.price
                >> order.quantity
                >> order.sequence;

            order.side =
                static_cast<Side>(side);

            order.type =
                static_cast<OrderType>(type);

            order.remainingQuantity =
                order.quantity;

            order.status =
                OrderStatus::NEW;

            orders.push_back(order);
        }

        return orders;
    }


private:

    std::string filename;
};