#include "MatchingEngine.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using Clock = std::chrono::steady_clock;


// Convert nanoseconds into a readable value.
double nsToDouble(
    std::chrono::nanoseconds value
)
{
    return static_cast<double>(
        value.count()
    );
}


int main()
{
    constexpr uint64_t NUM_ORDERS = 5'000'000;

    MatchingEngine engine;

    std::vector<uint64_t> latencies;

    latencies.reserve(NUM_ORDERS);


    std::mt19937_64 rng(42);

    std::uniform_int_distribution<int64_t>
        priceDistribution(9900, 10100);


    std::cout
        << "Benchmarking "
        << NUM_ORDERS
        << " orders...\n";


    auto benchmarkStart =
        Clock::now();


    for (uint64_t i = 0;
         i < NUM_ORDERS;
         ++i)
    {
        // Generate a deterministic mix of BUY/SELL orders.
        Side side =
            (i % 2 == 0)
            ? Side::BUY
            : Side::SELL;


        Order order{
            i,
            "AAPL",
            side,
            OrderType::LIMIT,
            priceDistribution(rng),
            1,
            i,
            1,
            OrderStatus::NEW
        };


        auto start =
            Clock::now();


        engine.submitOrder(order);


        auto end =
            Clock::now();


        latencies.push_back(
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(end - start).count()
        );
    }


    auto benchmarkEnd =
        Clock::now();


    auto totalTime =
        std::chrono::duration_cast<
            std::chrono::nanoseconds
        >(benchmarkEnd - benchmarkStart);


    // Sort so we can calculate latency percentiles.
    std::sort(
        latencies.begin(),
        latencies.end()
    );


    uint64_t p50 =
        latencies[
            NUM_ORDERS * 50 / 100
        ];

    uint64_t p95 =
        latencies[
            NUM_ORDERS * 95 / 100
        ];

    uint64_t p99 =
        latencies[
            NUM_ORDERS * 99 / 100
        ];


    double seconds =
        totalTime.count() / 1'000'000'000.0;


    double throughput =
        NUM_ORDERS / seconds;


    std::cout << "\n========== BENCHMARK ==========\n";

    std::cout
        << "Orders:      "
        << NUM_ORDERS
        << "\n";

    std::cout
        << "Time:        "
        << seconds
        << " sec\n";

    std::cout
        << "Throughput:  "
        << throughput
        << " orders/sec\n";

    std::cout
        << "P50:         "
        << p50
        << " ns\n";

    std::cout
        << "P95:         "
        << p95
        << " ns\n";

    std::cout
        << "P99:         "
        << p99
        << " ns\n";

    std::cout
        << "===============================\n";


    return 0;
}