#include "ConcurrentMatchingEngine.hpp"

#include "httplib.h"

#include <iostream>
#include <string>


class ApiServer
{
public:

    ApiServer(
        ConcurrentMatchingEngine& engine
    )
        : engine(engine)
    {
    }


    void start()
    {
        httplib::Server server;


        // --------------------------------------------------
        // Health check
        //
        // GET /health
        //
        // Useful for checking whether the server is alive.
        // --------------------------------------------------

        server.Get(
            "/health",
            [](const httplib::Request&,
               httplib::Response& response)
            {
                response.set_content(
                    R"({"status":"ok"})",
                    "application/json"
                );
            }
        );


        // --------------------------------------------------
        // Submit order
        //
        // POST /orders
        //
        // Example body:
        //
        // {
        //   "id": 100,
        //   "instrument": "AAPL",
        //   "side": "BUY",
        //   "type": "LIMIT",
        //   "price": 10000,
        //   "quantity": 500
        // }
        // --------------------------------------------------

        server.Post(
            "/orders",
            [this](
                const httplib::Request& request,
                httplib::Response& response)
            {
                std::cout
                    << "Received order request\n";


                // For the first version we're keeping
                // request parsing intentionally simple.
                //
                // JSON parsing will be added after the
                // endpoint itself is working.


                response.set_content(
                    R"({"status":"received"})",
                    "application/json"
                );
            }
        );


        std::cout
            << "HTTP server listening on "
            << "http://localhost:8080\n";


        server.listen(
            "0.0.0.0",
            8080
        );
    }


private:

    ConcurrentMatchingEngine& engine;
};