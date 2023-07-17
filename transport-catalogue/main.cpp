/*
int main() {

    TC::TransportCatalogue catalogue;
    TC::MapRenderer renderer;

    TC::RequestHandler request_handler(catalogue, renderer);

    TC::Input::JSONReader reader(std::cin);
    
    request_handler.ReadRequests(std::cout, reader, json::Builder{});

    return 0;
}*/

#include <fstream>
#include <iostream>
#include <string_view>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "json_builder.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        TC::TransportCatalogue catalogue;
        TC::MapRenderer renderer;

        TC::RequestHandler request_handler(catalogue, renderer);

        TC::Input::JSONReader reader(std::cin);
        
        request_handler.ReadRequests(std::cout, reader, json::Builder{});

    } else if (mode == "process_requests"sv) {

        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}