#pragma once

#include <istream>
#include "json.h"
#include "transport_catalogue.h"

namespace TC {

    namespace Input{

        namespace Json {

            void Reader(TransportCatalogue& catalogue, std::istream& input);
        }

    } // namespace JsonReader
} // namespace TC