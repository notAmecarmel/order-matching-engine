#pragma once

#include "Order.hpp"

#include <fstream>
#include <string>
#include <vector>

enum class EventType
{
    ORDER,
    CANCEL
};

struct LogEvent
{
    EventType type;
    Order order;
    uint64_t orderId;
};


class EventLog
{
public:

    explicit EventLog(const std::string& filename)
        : filename(filename)
    {
    }


    void appendOrder(const Order& order)
    {
        std::ofstream file(filename, std::ios::app);

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


    void appendCancel(uint64_t orderId)
    {
        std::ofstream file(filename, std::ios::app);

        file
            << "CANCEL "
            << orderId
            << "\n";
    }


    std::vector<LogEvent> replay()
    {
        std::vector<LogEvent> events;

        std::ifstream file(filename);

        std::string type;

        while (file >> type)
        {
            if (type == "ORDER")
            {
                Order order;

                int side;
                int orderType;

                file
                    >> order.id
                    >> order.instrument
                    >> side
                    >> orderType
                    >> order.price
                    >> order.quantity
                    >> order.sequence;

                order.side =
                    static_cast<Side>(side);

                order.type =
                    static_cast<OrderType>(orderType);

                order.remainingQuantity =
                    order.quantity;

                order.status =
                    OrderStatus::NEW;

                events.push_back({
                    EventType::ORDER,
                    order,
                    0
                });
            }
            else if (type == "CANCEL")
            {
                uint64_t orderId;

                file >> orderId;

                events.push_back({
                    EventType::CANCEL,
                    {},
                    orderId
                });
            }
        }

        return events;
    }


private:

    std::string filename;
};