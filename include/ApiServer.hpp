#pragma once

#include "ConcurrentMatchingEngine.hpp"

#include "httplib.h"

class ApiServer
{
public:

    explicit ApiServer(
        ConcurrentMatchingEngine& engine
    );

    void start();

private:

    ConcurrentMatchingEngine& engine;
};