#include "stat_reader.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <sstream>
#include <fstream>
#include <string>

int main() {

    //TC::Input::Reader(catalogue, std::cin);
    //TC::Output::StatReader(catalogue, std::cin, std::cout);

    std::ifstream in("../transport-catalogue/input.json", std::ios_base::in);

    if(!in.is_open())
        std::cout << "failed to open" << std::endl;

    TC::TransportCatalogue catalogue;
    TC::RequestHandler request_handler(catalogue);

    const auto document =  TC::Input::Json::Reader(request_handler, in);

    TC::Input::Json::ReadStatRequests(request_handler, document, std::cout);

    auto MapRenderSettings = TC::Input::Json::ReadMapRenderSettings(document);

    //TC::Input::Json::Reader(catalogue, std::cin, std::cout);

    return 0;
}