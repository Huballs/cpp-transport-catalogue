#pragma once

#include <transport_catalogue.pb.h>
#include "domain.h"

namespace TC{

    void SeriallizeTC(const TransportCatalogue& catalogue, std::string_view file_name);

    void DeseriallizeTC(TransportCatalogue& catalogue, std::string_view file_name);
}