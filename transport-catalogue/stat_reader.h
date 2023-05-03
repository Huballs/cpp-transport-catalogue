#pragma once

#include <iostream>
#include "transport_catalogue.h"

namespace TC {
    namespace Output {
        
        void StatReader(TransportCatalogue& catalogue, std::istream& input, std::ostream& output);
    }
}