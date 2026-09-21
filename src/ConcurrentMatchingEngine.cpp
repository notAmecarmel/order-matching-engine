#include "ConcurrentMatchingEngine.hpp"


ConcurrentMatchingEngine::ConcurrentMatchingEngine()
    : running(true)
{
    matcherThread =
        std::thread(
            &ConcurrentMatchingEngine::matchingLoop,
            this
        );
}


ConcurrentMatchingEngine::~ConcurrentMatchingEngine()
{
    running = false;

    // Wake the matcher if it is waiting.
    orderQueue.stop();

    if (matcherThread.joinable())
    {
        matcherThread.join();
    }
}


void ConcurrentMatchingEngine::submit(
    const Order& order
)
{
    orderQueue.push(order);
}


void ConcurrentMatchingEngine::matchingLoop()
{
    while (running)
    {
        Order order;

        // Wait for the next order.
        if (!orderQueue.pop(order))
        {
            break;
        }

        // IMPORTANT:
        // Only this thread modifies the order books.
        engine.submitOrder(order);
    }
}