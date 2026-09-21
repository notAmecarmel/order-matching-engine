/*#include "ConcurrentMatchingEngine.hpp"

#include <iostream>

int main()
{
    {
        ConcurrentMatchingEngine engine;

        engine.submit({
            1,
            "AAPL",
            Side::BUY,
            OrderType::LIMIT,
            10000,
            500,
            1,
            500,
            OrderStatus::NEW
        });

        engine.submit({
            2,
            "AAPL",
            Side::SELL,
            OrderType::LIMIT,
            10500,
            200,
            2,
            200,
            OrderStatus::NEW
        });
    }


    std::cout << "\nOriginal engine shut down.\n";


    // Start a completely new engine.
    ConcurrentMatchingEngine recoveredEngine;

    // Rebuild its state from orders.log.
    recoveredEngine.recover();


    std::cout << "\nRecovery completed.\n";

    return 0;
}*/

#include "ApiServer.cpp"

int main()
{
    ConcurrentMatchingEngine engine;

    ApiServer server(engine);

    server.start();

    return 0;
}