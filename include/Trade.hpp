#pragma once

#include <cstdint>
#include <string>

struct Trade {
    uint64_t buyOrderId;
    uint64_t sellOrderId;

    std::string instrument;

    int64_t price;
    uint64_t quantity;
};