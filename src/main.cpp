#include "ConcurrentMatchingEngine.hpp"

#include <iostream>
#include <thread>
#include <vector>


int main()
{
    ConcurrentMatchingEngine engine;


    constexpr int NUM_THREADS = 4;
    constexpr int ORDERS_PER_THREAD = 25000;


    std::vector<std::thread> producers;


    // Start multiple producer threads.
    for (int threadId = 0;
         threadId < NUM_THREADS;
         ++threadId)
    {
        producers.emplace_back(
            [&engine, threadId]()
            {
                for (int i = 0;
                     i < ORDERS_PER_THREAD;
                     ++i)
                {
                    uint64_t orderId =
                        static_cast<uint64_t>(
                            threadId * ORDERS_PER_THREAD + i
                        );


                    Order order{
                        orderId,
                        "AAPL",

                        Side::BUY,
                        OrderType::LIMIT,

                        10000,

                        1,

                        orderId,

                        1,

                        OrderStatus::NEW
                    };


                    // Multiple threads are simultaneously
                    // submitting orders.
                    engine.submit(order);
                }
            }
        );
    }


    // Wait for every producer to finish.
    for (auto& producer : producers)
    {
        producer.join();
    }


    std::cout
        << "Submitted "
        << NUM_THREADS * ORDERS_PER_THREAD
        << " orders concurrently.\n";


    // Engine destructor will shut down
    // the matcher thread cleanly.

    return 0;
}