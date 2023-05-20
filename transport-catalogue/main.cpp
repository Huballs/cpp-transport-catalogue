#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"

#include <iostream>

int main() {

    TC::TransportCatalogue catalogue;
    TC::Input::JSONReader reader(std::cin);
    TC::MapRenderer renderer;

    TC::RequestHandler request_handler(catalogue, renderer);

    request_handler.ReadRequests(std::cout, reader);

    //const auto document =  TC::Input::Json::Reader(request_handler, std::cin);

    //TC::Input::Json::ReadStatRequests(request_handler, document, std::cout);

    return 0;
}