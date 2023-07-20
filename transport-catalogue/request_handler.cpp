#include "request_handler.h"
#include "serialization.h"
#include <algorithm>

namespace TC {

    void RequestHandler::SerializeToFile(std::string_view file_name){
        TC_PROTO::TransportCatalogue* proto_transport = TransportCatalogueToProto(db_);
        TC_PROTO::RenderSettings* proto_render_settings = RenderSettingsToProto(render_settings_);

        TC_PROTO::BusManager bus_manager;

        bus_manager.set_allocated_transport_catalogue(proto_transport);
        bus_manager.set_allocated_render_settings(proto_render_settings);

        std::ofstream file(std::string(file_name), std::ios::binary);

        bus_manager.SerializeToOstream(&file);
    }

    void RequestHandler::QueueAddRequest(base_request_t& request) {

        if (request.type == "Stop"){
            requests_add_stop.push_back(std::move(request));
        } else 
        if (request.type == "Bus"){
            requests_add_bus.push_back(std::move(request));
        }
    }

    void RequestHandler::FulfillAddRequests(){

        for(const auto& request : requests_add_stop){
            db_.AddStop(request.name, {request.latitude, request.longitude});
            
        }

        for(const auto& request : requests_add_stop){
            db_.AddDistances(request.name, request.road_distances);
        }

        for(const auto& request : requests_add_bus){
            std::vector<size_t> indexed_stops;
            indexed_stops.reserve(request.stops.size());

            for(auto stop : request.stops){
                indexed_stops.push_back(db_.GetStop(stop)->GetIndex());
            }
            db_.AddRoute(request.name, indexed_stops, request.is_roundtrip);
        }

    }

    std::optional<RequestHandler::stat_bus_t> RequestHandler::GetBusStat(const std::string_view& bus_name) const {

        if (!db_.ContainsBus(bus_name))
            return std::nullopt;

        const auto bus = db_.GetBus(bus_name);

        stat_bus_t bus_stat;

        bus_stat.curvature = bus->GetCurvature();
        bus_stat.length = bus->GetLengthTraveled();
        bus_stat.stop_count = bus->GetStopsCount();
        bus_stat.unique_stop_count = bus->GetStopsCountUnique();

        return bus_stat;
    }

    std::optional<RequestHandler::stat_stop_t> RequestHandler::GetStopStat(const std::string_view& stop_name) const {
        
        if (!db_.ContainsStop(stop_name))
            return std::nullopt;

        const auto stop = db_.GetStop(stop_name);

        stat_stop_t stop_stat{stop->GetBuses()};

        return stop_stat;

    }

    std::vector<const Bus*> RequestHandler::GetBusesAscendingName() const {

        std::vector<const Bus*> result;

        for(const auto& bus : db_.GetBuses()){
            result.push_back(&bus);
        }

        std::sort(result.begin(), result.end(), [](const Bus* lhs,const Bus* rhs){
            return lhs->GetName() < rhs->GetName();
        });

        return result;
    }

    std::vector<const Stop*> RequestHandler::GetStopsAscendingName() const {

        std::vector<const Stop*> result;

        for(const auto& stop : db_.GetStops()){
            result.push_back(&stop);
        }

        std::sort(result.begin(), result.end(), [](const Stop* lhs,const Stop* rhs){
            return lhs->GetName() < rhs->GetName();
        });

        return result;
    }

    const std::deque<Bus>& RequestHandler::GetBuses() const{
        return db_.GetBuses();
    }


} // TC

