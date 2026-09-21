# Order Matching Engine

A high-performance C++ order matching engine implementing **price-time priority**, limit and market orders, partial fills, cancellations, multi-instrument order books, concurrent order ingestion, and event logging with recovery support.

The project is designed to explore the core systems concepts behind electronic trading infrastructure: deterministic matching, data structures, concurrency, persistence, and low-latency processing.

---

## Features

* **Price-time priority**

  * Orders are matched according to best price first.
  * Orders at the same price are matched FIFO based on arrival sequence.

* **Limit orders**

  * Buy orders execute at or below their limit price.
  * Sell orders execute at or above their limit price.
  * Unfilled quantities rest on the order book.

* **Market orders**

  * Execute immediately against available liquidity.
  * Consume the best available price levels.
  * Unfilled quantities do not rest on the book.

* **Partial fills**

  * Orders can be matched across multiple counterparties and price levels.
  * Remaining quantities are tracked independently.

* **Order cancellation**

  * Resting orders can be cancelled using their order ID.
  * Price levels are automatically removed when empty.

* **Multi-instrument support**

  * Each instrument maintains an independent order book.
  * Example: `AAPL`, `TSLA`, etc.

* **Concurrent order ingestion**

  * Multiple producer threads can submit orders concurrently.
  * A dedicated matching thread serializes state changes.
  * This preserves deterministic order-book state while allowing concurrent ingestion.

* **Event logging**

  * Incoming orders and cancellations are persisted to an append-only log.
  * Events can be replayed to reconstruct in-memory state.

* **Crash recovery**

  * The engine can replay persisted events to rebuild its order books after restart.

---

## Architecture

```text
                    ┌──────────────────┐
                    │   Producer 1     │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │   Producer 2     │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │   Producer 3     │
                    └────────┬─────────┘
                             │
                             ▼
                 ┌────────────────────────┐
                 │   Thread-Safe Queue    │
                 │  mutex + condition var │
                 └────────────┬───────────┘
                              │
                              ▼
                 ┌────────────────────────┐
                 │    Matcher Thread      │
                 │   Serialized Matching  │
                 └────────────┬───────────┘
                              │
                 ┌────────────▼───────────┐
                 │    Matching Engine     │
                 └────────────┬───────────┘
                              │
             ┌────────────────┼────────────────┐
             ▼                ▼                ▼
        ┌──────────┐     ┌──────────┐     ┌──────────┐
        │  AAPL    │     │  TSLA    │     │  Other   │
        │OrderBook │     │OrderBook │     │Instruments│
        └──────────┘     └──────────┘     └──────────┘

                              │
                              ▼
                     ┌────────────────┐
                     │   Event Log    │
                     │   orders.log   │
                     └────────────────┘
```

### Why serialized matching?

The order book is shared mutable state. Allowing multiple threads to modify it simultaneously would introduce race conditions and make matching nondeterministic.

Instead, the system separates:

**Concurrent ingestion**

```text
Multiple producer threads
        ↓
Thread-safe queue
```

from:

**Deterministic matching**

```text
Single matcher thread
        ↓
OrderBook
```

This allows the system to accept orders concurrently while maintaining deterministic state transitions.

---

## Order Book Design

Each instrument has its own `OrderBook`.

The book contains two sides:

```text
BUY                         SELL

Higher prices first         Lower prices first

10500 → [Order A]           10600 → [Order C]
10400 → [Order B]           10700 → [Order D]
10300 → [Order E]           10800 → [Order F]
```

### Data structures

The engine uses:

```cpp
std::map<int64_t, std::deque<Order>>
```

for each side of the book.

### BUY side

The BUY book uses a descending comparator:

```cpp
std::map<
    int64_t,
    std::deque<Order>,
    std::greater<int64_t>
>
```

This means the highest bid is always:

```cpp
buyOrders.begin()
```

### SELL side

The SELL book uses the default ascending ordering:

```cpp
std::map<
    int64_t,
    std::deque<Order>
>
```

Therefore the lowest ask is always:

```cpp
sellOrders.begin()
```

### Why `deque`?

Each price level contains a FIFO queue:

```text
Price: 10000

Order 1
Order 2
Order 3
```

Order 1 must receive matching priority before Order 2, and Order 2 before Order 3.

This implements the **time-priority** component of price-time priority.

---

## Matching Logic

### Incoming BUY

The engine checks the cheapest SELL order.

For a limit order:

```text
BUY price >= SELL price
```

For a market order:

```text
Match regardless of price
```

The engine continues consuming liquidity until:

* The incoming order is completely filled.
* No more compatible liquidity exists.
* A limit order reaches a price level it cannot cross.

### Incoming SELL

The same logic is reversed:

```text
SELL price <= BUY price
```

for limit orders.

Market sells consume the highest available bids.

### Trade price

Trades execute at the price of the resting order.

Example:

```text
Resting SELL: 10000
Incoming BUY: 10100
```

The trade occurs at:

```text
10000
```

---

## Partial Fills

Suppose the book contains:

```text
SELL 10000 → 50
SELL 10100 → 100
```

and a market BUY for 120 arrives.

The engine produces:

```text
50 @ 10000
70 @ 10100
```

The second sell order has:

```text
30 remaining
```

and remains on the book.

---

## Price Representation

Prices are represented using integer ticks rather than floating-point values.

For example:

```text
₹100.00 → 10000
₹101.50 → 10150
```

This avoids floating-point precision problems when dealing with financial values.

---

## Order Lifecycle

Orders move through states such as:

```text
NEW
 │
 ▼
RESTING
 │
 ├──────────────┐
 ▼              ▼
PARTIALLY     CANCELLED
FILLED
 │
 ▼
FILLED
```

Market orders may go directly from:

```text
NEW → FILLED
```

or, if insufficient liquidity exists:

```text
NEW → PARTIALLY_FILLED
```

with the unfilled remainder discarded.

---

## Event Logging

Orders are persisted to an append-only event log.

Example:

```text
ORDER 1 AAPL 0 0 10000 500 1
ORDER 2 AAPL 1 0 10100 200 2
CANCEL 1
```

The event log allows the engine to reconstruct state after restarting.

### Recovery

During recovery:

```text
orders.log
     ↓
Read events
     ↓
Replay in original order
     ↓
Rebuild order books
```

This follows an event-replay model rather than storing only a snapshot of the current order book.

---

## Project Structure

```text
order-matching-engine/
│
├── include/
│   ├── Order.hpp
│   ├── Trade.hpp
│   ├── OrderBook.hpp
│   ├── MatchingEngine.hpp
│   ├── OrderQueue.hpp
│   ├── ConcurrentMatchingEngine.hpp
│   └── EventLog.hpp
│
├── src/
│   ├── main.cpp
│   ├── OrderBook.cpp
│   ├── MatchingEngine.cpp
│   └── ConcurrentMatchingEngine.cpp
│
├── tests/
│
├── CMakeLists.txt
│
└── README.md
```

---

## Tech Stack

* **C++20**
* **CMake**
* **STL**

  * `std::map`
  * `std::unordered_map`
  * `std::deque`
  * `std::queue`
  * `std::thread`
  * `std::mutex`
  * `std::condition_variable`
  * `std::atomic`
* **Linux/macOS**
* **Docker** *(planned integration)*

---

## Building

### Requirements

* C++20-compatible compiler
* CMake 3.20+
* Make

### Build

```bash
git clone <repository-url>

cd order-matching-engine

mkdir -p build
cd build

cmake ..
make
```

### Run

```bash
./order_engine
```

---

## Example

Submitting a BUY limit order:

```cpp
Order order{
    100,
    "AAPL",
    Side::BUY,
    OrderType::LIMIT,
    10000,
    500,
    1,
    500,
    OrderStatus::NEW
};

engine.submit(order);
```

A matching SELL order:

```cpp
Order order{
    200,
    "AAPL",
    Side::SELL,
    OrderType::LIMIT,
    10000,
    100,
    2,
    100,
    OrderStatus::NEW
};
```

produces a trade:

```text
BUY 100
SELL 200
Price 10000
Quantity 100
```

The remaining BUY quantity stays on the order book:

```text
BUY 10000 → 400
```

---

## Testing

The project includes tests for:

* Limit order matching
* Market order matching
* Partial fills
* Multiple price levels
* FIFO time priority
* Order cancellation
* Multiple instruments
* Concurrent order submission
* Event logging
* Event replay / recovery

A concurrent stress test can submit orders from multiple producer threads:

```text
4 producer threads
       ↓
100,000 orders
       ↓
thread-safe queue
       ↓
single matcher
```

---

## Performance Benchmarking

A dedicated benchmark is planned to measure:

* Orders processed per second
* Average matching latency
* P50 latency
* P95 latency
* P99 latency
* Throughput under concurrent ingestion

Benchmark results should be reported from actual runs on the target machine rather than hard-coded claims.

Example:

```text
Orders:        5,000,000
Throughput:    X orders/sec
P50 latency:   X ns
P99 latency:   X ns
```

---

## Design Decisions

### `std::map` instead of `unordered_map`

Price levels need to remain ordered so the engine can efficiently locate the best bid and ask.

### `std::deque` instead of `vector`

Orders at a price level are consumed from the front, making FIFO operations natural.

### Single matching thread

Serializing modifications to the order book avoids complex locking inside the matching algorithm and preserves deterministic behavior.

### Integer prices

Avoids floating-point precision problems in financial calculations.

### Event log

Provides a foundation for persistent state and crash recovery without requiring the entire order book to be serialized after every operation.

---

## Future Improvements

* REST API for order submission and cancellation
* WebSocket market-data stream
* Full trade-event persistence
* Exact replay of fills and order lifecycle transitions
* Snapshot + WAL recovery
* Lock-free or bounded MPSC queues
* More comprehensive unit and integration tests
* 5M+ order benchmark suite
* Latency percentile reporting
* Dockerized Linux deployment
* Metrics and observability
* Order-book market data feed
* Improved memory allocation strategy
* Batch processing benchmarks

---

## Learning Goals

This project was built to understand the systems concepts behind an electronic matching engine rather than treating it as a simple CRUD application.

Key concepts explored:

* Data structure selection
* Price-time priority
* Matching algorithms
* FIFO queues
* Partial fills
* Concurrent producers
* Serialized state transitions
* Thread synchronization
* Event logging
* State reconstruction
* Deterministic processing
* Performance benchmarking
* Systems-oriented C++ design

---

## Disclaimer

This is an educational systems project and is **not intended for production trading or use with real financial markets**.
