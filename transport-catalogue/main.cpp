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
#include "serialization.h"

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
        TC::RequestHandler request_handler(catalogue);
        TC::Input::JSONReader reader(std::cin);
        
        request_handler.ReadRequests(reader);

        auto file_name = request_handler.ReadSerializationSettings(reader);

        TC::SeriallizeTC(catalogue, file_name);

    } else if (mode == "process_requests"sv) {

        TC::TransportCatalogue catalogue;
        TC::RequestHandler request_handler(catalogue);
        TC::Input::JSONReader reader(std::cin);

        auto file_name = request_handler.ReadSerializationSettings(reader);

        TC::DeseriallizeTC(catalogue, file_name);

        request_handler.ReadStatRequests(std::cout, reader, json::Builder{});

    } else {
        PrintUsage();
        return 1;
    }
}