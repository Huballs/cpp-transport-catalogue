#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <sstream>
#include <fstream>
#include <string>

int main() {
    
    TC::TransportCatalogue catalogue;

    //TC::Input::Reader(catalogue, std::cin);
    //TC::Output::StatReader(catalogue, std::cin, std::cout);

    std::ifstream in("../transport-catalogue/input.json", std::ios_base::in);

    if(!in.is_open())
        std::cout << "failed to open" << std::endl;

    auto doc = LoadJson(in);

    doc.GetRoot().AsArray()

    std::string line;
    std::string text;
    while(std::getline(in, line)){
        text += line + '\n';
    }

    std::cout << text << std::endl;

    return 0;
}