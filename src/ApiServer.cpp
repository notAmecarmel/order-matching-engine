#include "ApiServer.hpp"

#include "json.hpp"

#include <iostream>

using json = nlohmann::json;


ApiServer::ApiServer(
    ConcurrentMatchingEngine& engine
)
    : engine(engine)
{
}


void ApiServer::start()
{
    httplib::Server server;


    // --------------------------------------------------
    // HEALTH CHECK
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
    // SUBMIT ORDER
    //
    // POST /orders
    //
    // JSON:
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
            try
            {
                // Convert HTTP body into JSON.
                json data =
                    json::parse(request.body);


                uint64_t id =
                    data.at("id");

                std::string instrument =
                    data.at("instrument");

                std::string sideString =
                    data.at("side");

                std::string typeString =
                    data.at("type");

                int64_t price =
                    data.at("price");

                uint64_t quantity =
                    data.at("quantity");


                Side side;

                if (sideString == "BUY")
                {
                    side = Side::BUY;
                }
                else if (sideString == "SELL")
                {
                    side = Side::SELL;
                }
                else
                {
                    response.status = 400;

                    response.set_content(
                        R"({"error":"Invalid side"})",
                        "application/json"
                    );

                    return;
                }


                OrderType type;

                if (typeString == "LIMIT")
                {
                    type = OrderType::LIMIT;
                }
                else if (typeString == "MARKET")
                {
                    type = OrderType::MARKET;
                }
                else
                {
                    response.status = 400;

                    response.set_content(
                        R"({"error":"Invalid order type"})",
                        "application/json"
                    );

                    return;
                }


                Order order{
                    id,
                    instrument,
                    side,
                    type,
                    price,
                    quantity,
                    id,
                    quantity,
                    OrderStatus::NEW
                };


                // Send the order into the
                // concurrent ingestion pipeline.
                engine.submit(order);


                json result = {
                    {"status", "accepted"},
                    {"order_id", id}
                };


                response.set_content(
                    result.dump(),
                    "application/json"
                );
            }
            catch (const std::exception& error)
            {
                response.status = 400;

                json result = {
                    {"error", error.what()}
                };

                response.set_content(
                    result.dump(),
                    "application/json"
                );
            }
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