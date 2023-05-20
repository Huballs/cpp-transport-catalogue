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

    return 0;
}