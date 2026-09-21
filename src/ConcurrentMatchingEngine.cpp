#include "ConcurrentMatchingEngine.hpp"


ConcurrentMatchingEngine::ConcurrentMatchingEngine()
    : eventLog("orders.log"),
      running(true)
{
    // Start one dedicated thread that owns the
    // actual matching process.
    matcherThread =
        std::thread(
            &ConcurrentMatchingEngine::matchingLoop,
            this
        );
}


ConcurrentMatchingEngine::~ConcurrentMatchingEngine()
{
    // Tell the matcher to stop.
    running = false;

    // Wake it up if it is currently waiting
    // for an order.
    orderQueue.stop();

    // Wait for the matcher thread to finish.
    if (matcherThread.joinable())
    {
        matcherThread.join();
    }
}


void ConcurrentMatchingEngine::submit(
    const Order& order
)
{
    // Multiple threads can safely call this.
    orderQueue.push(order);
}


void ConcurrentMatchingEngine::matchingLoop()
{
    while (running)
    {
        Order order;

        // Wait for an order from the producer queue.
        if (!orderQueue.pop(order))
        {
            break;
        }

        // Persist the order BEFORE modifying
        // the in-memory state.
        eventLog.appendOrder(order);

        // Only this thread touches the matching engine.
        engine.submitOrder(order);
    }
}

void ConcurrentMatchingEngine::recover()
{
    // Read all orders that were persisted
    // before the previous shutdown/crash.
    auto orders = eventLog.replay();

    for (const auto& order : orders)
    {
        // Rebuild the in-memory order book.
        engine.submitOrder(order);
    }
}