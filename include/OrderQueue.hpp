#pragma once

#include "Order.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>

class OrderQueue
{
public:

    void push(const Order& order)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(order);
        }

        condition.notify_one();
    }


    bool pop(Order& order)
    {
        std::unique_lock<std::mutex> lock(mutex);

        // Wait until either:
        // 1. An order arrives
        // 2. The queue is shutting down
        condition.wait(
            lock,
            [this]
            {
                return !queue.empty() || stopped;
            }
        );

        // Nothing left to process.
        if (queue.empty())
        {
            return false;
        }

        order = queue.front();
        queue.pop();

        return true;
    }


    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            stopped = true;
        }

        // Wake the matcher thread so it can exit.
        condition.notify_all();
    }


private:

    std::queue<Order> queue;

    std::mutex mutex;

    std::condition_variable condition;

    bool stopped = false;
};