#include "serialization.h"

namespace TC {

        namespace detail {

        inline TC_PROTO::Stop* StopToProto(const Stop& stop){
            TC_PROTO::Stop* proto_stop = new TC_PROTO::Stop();
            proto_stop->set_id(stop.GetIndex());
            proto_stop->set_name(stop.GetNameStr());
            proto_stop->set_latitude(stop.getCoordinates().lat);
            proto_stop->set_longitude(stop.getCoordinates().lng);

            return proto_stop;
        }

        inline TC_PROTO::Bus* BusToProto(const Bus& bus){
            TC_PROTO::Bus* proto_bus = new TC_PROTO::Bus();

            proto_bus->set_name(bus.GetNameStr());
            proto_bus->set_is_roundtrip(bus.IsCircular());

            for(const Stop* stop : bus.GetStops()){
                proto_bus->mutable_stops()->Add(stop->GetIndex());
            }

            return proto_bus;

        }

        inline TC_PROTO::Distance* StopsDistanceToProto(const std::pair<TC::Stop *, TC::Stop *>& stops, uint32_t distance){
            TC_PROTO::Distance* proto_dist = new TC_PROTO::Distance();

            proto_dist->set_start(stops.first->GetIndex());
            proto_dist->set_end(stops.second->GetIndex());
            proto_dist->set_distance(distance);

            return proto_dist;
        }

        inline TC_PROTO::Point* CreateProtoPoint(const svg::Point& point){
            TC_PROTO::Point* result = new TC_PROTO::Point();
            result->set_x(point.x);
            result->set_y(point.y);
            return result;
        }

        inline TC_PROTO::Color* CreateProtoColor(const svg::Color& color){
            TC_PROTO::Color* result = new TC_PROTO::Color();

            if(std::holds_alternative<std::monostate>(color)){
                result->set_none(true);
            } else
            if(std::holds_alternative<svg::Rgb>(color)){
                const auto& rgb = std::get<svg::Rgb>(color);
                TC_PROTO::Rgb* rgb_proto = new TC_PROTO::Rgb();
                rgb_proto->set_red(rgb.red);
                rgb_proto->set_green(rgb.green);
                rgb_proto->set_blue(rgb.blue);

                result->set_allocated_rgb(rgb_proto);
            } else
            if (std::holds_alternative<svg::Rgba>(color)){
                const auto& rgba = std::get<svg::Rgba>(color);
                TC_PROTO::Rgba* rgba_proto = new TC_PROTO::Rgba();
                rgba_proto->set_red(rgba.red);
                rgba_proto->set_green(rgba.green);
                rgba_proto->set_blue(rgba.blue);
                rgba_proto->set_opacity(rgba.opacity);

                result->set_allocated_rgba(rgba_proto);
            } else
            if(std::holds_alternative<std::string>(color)){
                const auto& string_color = std::get<std::string>(color);
                result->set_string_color(string_color);
            } 
            else{
                result->set_none(true);
            }

            return result;
        }
    }

    TC_PROTO::RenderSettings* RenderSettingsToProto(const map_settings_t& settings){
        using namespace detail; 

        TC_PROTO::RenderSettings* settings_proto = new TC_PROTO::RenderSettings();

        settings_proto->set_width(settings.width);
        settings_proto->set_height(settings.height);
        settings_proto->set_padding(settings.padding);
        settings_proto->set_line_width(settings.line_width);
        settings_proto->set_stop_radius(settings.stop_radius);
        settings_proto->set_bus_label_font_size(settings.bus_label_font_size);
        settings_proto->set_allocated_bus_label_offset(CreateProtoPoint(settings.bus_label_offset));
        settings_proto->set_stop_label_font_size(settings.stop_label_font_size);
        settings_proto->set_allocated_stop_label_offset(CreateProtoPoint(settings.stop_label_offset));
        settings_proto->set_allocated_underlayer_color(CreateProtoColor(settings.underlayer_color));
        settings_proto->set_underlayer_width(settings.underlayer_width);
        
        for(const auto& color : settings.color_palette){
            settings_proto->mutable_color_palette()->AddAllocated(CreateProtoColor(color));
        }

        return settings_proto;

    }

    TC_PROTO::TransportCatalogue* TransportCatalogueToProto(const TransportCatalogue& catalogue){
        TC_PROTO::TransportCatalogue* proto_catalogue = new TC_PROTO::TransportCatalogue();

        using namespace detail;

        for(const auto& stop : catalogue.GetStops()){
            proto_catalogue->mutable_stops()->AddAllocated(StopToProto(stop));
        }

        for(const auto& bus : catalogue.GetBuses()){
            proto_catalogue->mutable_buses()->AddAllocated(BusToProto(bus));
        }

        for(const auto& [stops, dist] : catalogue.GetStopsDistances()){
            proto_catalogue->mutable_distances()->AddAllocated(StopsDistanceToProto(stops, dist));
        }

        //std::ofstream file(std::string(file_name), std::ios::binary);

       // proto_catalogue.SerializeToOstream(&file);
       return proto_catalogue;

    }

    void DeseriallizeTC(TransportCatalogue& catalogue, std::string_view file_name){
        TC_PROTO::TransportCatalogue proto_catalogue;

        std::ifstream file(std::string(file_name), std::ios::binary);

        proto_catalogue.ParseFromIstream(&file);

        for(const auto& stop : proto_catalogue.stops()){
            catalogue.AddStop(stop.name(), {stop.latitude(), stop.longitude()});
        }

        for(const auto& dist : proto_catalogue.distances()){
            catalogue.AddDistance(dist.start(), dist.end(), dist.distance());
        }

        for(const auto& bus : proto_catalogue.buses()){
            std::vector<size_t> indexed_stops;
            indexed_stops.reserve(bus.stops_size());

            for(size_t index : bus.stops()){
                indexed_stops.push_back(index);
            }

            catalogue.AddRoute(bus.name(), indexed_stops, bus.is_roundtrip());
        }
        
    }

}