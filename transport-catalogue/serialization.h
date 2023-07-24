#pragma once

#include <transport_catalogue.pb.h>
#include <fstream>

#include "domain.h"
#include "transport_router.h"

namespace TC{

    TC_PROTO::RenderSettings* RenderSettingsToProto(const map_settings_t& settings);

    TC_PROTO::TransportCatalogue* TransportCatalogueToProto(const TransportCatalogue& catalogue);

    TC_PROTO::TransportRouter* TransportRouterToProto(const TransportRouter& transport_router);

    void DeseriallizeBusManager(TransportCatalogue& catalogue, map_settings_t& render_settings, TransportRouter& router, std::istream& in_stream);
}