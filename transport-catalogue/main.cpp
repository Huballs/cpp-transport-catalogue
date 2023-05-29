#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "json_builder.h"
#include <iostream>

int main() {

    TC::TransportCatalogue catalogue;
    TC::MapRenderer renderer;

    TC::RequestHandler request_handler(catalogue, renderer);

    TC::Input::JSONReader reader(std::cin);
    
    request_handler.ReadRequests(std::cout, reader, json::Builder{});

    return 0;
}