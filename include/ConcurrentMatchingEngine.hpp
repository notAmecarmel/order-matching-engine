#pragma once

#include "MatchingEngine.hpp"
#include "OrderQueue.hpp"

#include <atomic>
#include <thread>

class ConcurrentMatchingEngine
{
public:

    ConcurrentMatchingEngine();

    ~ConcurrentMatchingEngine();

    // Called by producer threads.
    void submit(const Order& order);

private:

    // Continuously consumes orders and sends them
    // to the normal single-threaded matching engine.
    void matchingLoop();

    OrderQueue orderQueue;

    MatchingEngine engine;

    std::thread matcherThread;

    std::atomic<bool> running;
};