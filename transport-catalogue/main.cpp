#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>

int main() {

    TC::TransportCatalogue catalogue;
    TC::RequestHandler request_handler(catalogue);

    const auto document =  TC::Input::Json::Reader(request_handler, std::cin);

    TC::Input::Json::ReadStatRequests(request_handler, document, std::cout);

    return 0;
}