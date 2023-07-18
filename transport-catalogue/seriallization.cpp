#include "seriallization.h"
#include <fstream>


#include <iostream>

namespace TC {

        namespace detail {

        inline TC_PROTO::Stop StopToProto(const Stop& stop){
            TC_PROTO::Stop proto_stop;
            proto_stop.set_id(stop.GetIndex());
            proto_stop.set_name(stop.GetNameStr());
            proto_stop.set_latitude(stop.getCoordinates().lat);
            proto_stop.set_longitude(stop.getCoordinates().lng);

            return proto_stop;
        }

        inline TC_PROTO::Bus BusToProto(const Bus& bus){
            TC_PROTO::Bus proto_bus;

            proto_bus.set_name(bus.GetNameStr());
            proto_bus.set_is_roundtrip(bus.IsCircular());

            for(const Stop* stop : bus.GetStops()){
                proto_bus.mutable_stops()->Add(stop->GetIndex());
            }

            return proto_bus;

        }

        inline TC_PROTO::Distance StopsDistanceToProto(const std::pair<TC::Stop *, TC::Stop *>& stops, uint32_t distance){
            TC_PROTO::Distance proto_dist;

            proto_dist.set_start(stops.first->GetIndex());
            proto_dist.set_end(stops.second->GetIndex());
            proto_dist.set_distance(distance);

            return proto_dist;
        }
    }

    void SeriallizeTC(const TransportCatalogue& catalogue, std::string_view file_name){
        TC_PROTO::TransportCatalogue proto_catalogue;

        using namespace detail;

        for(const auto& stop : catalogue.GetStops()){
            proto_catalogue.mutable_stops()->Add(StopToProto(stop));
        }

        for(const auto& bus : catalogue.GetBuses()){
            proto_catalogue.mutable_buses()->Add(BusToProto(bus));
        }

        for(const auto& [stops, dist] : catalogue.GetStopsDistances()){
            proto_catalogue.mutable_distances()->Add(StopsDistanceToProto(stops, dist));
        }

        std::ofstream file(std::string(file_name), std::ios::binary);

        proto_catalogue.SerializeToOstream(&file);

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
    }

}