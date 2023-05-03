#pragma once

#include <istream>
#include "transport_catalogue.h"

namespace TC {

    namespace Input {
        
        void Reader(TransportCatalogue& catalogue, std::istream& input);

        namespace detail {

            std::pair<std::string_view, std::string_view> SplitCommand(std::string_view line);
        }
    }
}
