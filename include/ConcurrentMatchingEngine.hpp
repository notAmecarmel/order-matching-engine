#pragma once

#include "EventLog.hpp"
#include "MatchingEngine.hpp"
#include "OrderQueue.hpp"

#include <atomic>
#include <thread>

class ConcurrentMatchingEngine
{
public:
    ConcurrentMatchingEngine();

    ~ConcurrentMatchingEngine();

    // Called by any producer thread.
    void submit(const Order& order);

    void recover();

private:
    // Runs on exactly one matcher thread.
    void matchingLoop();

    OrderQueue orderQueue;

    // The normal matching engine is only accessed
    // by the matcher thread.
    MatchingEngine engine;

    // Persists incoming orders for recovery.
    EventLog eventLog;

    std::thread matcherThread;

    std::atomic<bool> running;
};