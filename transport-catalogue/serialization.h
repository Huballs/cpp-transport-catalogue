#pragma once

#include <transport_catalogue.pb.h>
#include <fstream>

#include "domain.h"

namespace TC{

    TC_PROTO::RenderSettings* RenderSettingsToProto(const map_settings_t& settings);

    TC_PROTO::TransportCatalogue* TransportCatalogueToProto(const TransportCatalogue& catalogue);

    void DeseriallizeTC(TransportCatalogue& catalogue, std::string_view file_name);
}