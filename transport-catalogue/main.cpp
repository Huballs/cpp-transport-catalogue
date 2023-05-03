#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include <sstream>

int main() {
    
    TC::TransportCatalogue catalogue;

    TC::Input::Reader(catalogue, std::cin);
    TC::Output::StatReader(catalogue, std::cin, std::cout);

    return 0;
}